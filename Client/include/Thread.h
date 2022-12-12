#include "Common.h"
#include "Protocol.h"
#include "Player.h"
#include <vector>
#include <stdio.h>
#define LOCALPORT 9000

extern int g_myid;
std::vector<Player*> player_list(3, NULL);
DWORD WINAPI RecvThread(LPVOID arg);
PLAYER_INPUT_INFO p_input[3];
extern Framework* framework;


DWORD WINAPI RecvThread(LPVOID arg)// //Ŭ���̾�Ʈ���� Recv�� ���� �� �۾��� ����ϴ� �Լ�
{
	SOCKET sock = (SOCKET)arg;
	int my_id = -1;

	while (true) {
		char buf[2];
		DWORD recvByte{ 0 }, recvFlag{ 0 };
		//recv(sock, buf, sizeof(buf), MSG_WAITALL);
		int error_code = recv(sock, buf, sizeof(buf), 0);
		//if (error_code == SOCKET_ERROR) error_display("RecvSizeType");

		UCHAR size{ static_cast<UCHAR>(buf[0]) };
		UCHAR type{ static_cast<UCHAR>(buf[1]) };
		switch (type)
		{
		case SC_LOGIN: //�α���
		{
			char IdBuf[sizeof(short)]{};
			char subBuf[sizeof(LOGIN_INFO[3])]{};
			recv(sock, IdBuf, sizeof(IdBuf), 0);
			recv(sock, subBuf, sizeof(subBuf), 0);

			short id;
			LOGIN_INFO loginInfo[3];
			memcpy(&id, &IdBuf, sizeof(short));
			memcpy(&loginInfo, &subBuf, sizeof(LOGIN_INFO[3]));
			g_myid = my_id = id;

			printf("id : %d\n", my_id);

			for (int i = 0; i < PLAYER_NUM; ++i)
			{
				if (!player_list[i]) {
					player_list[i] = new Player;
				}
				char addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &loginInfo[i].ip, addr, sizeof(addr));
				printf("ip : %s\n", addr);
				player_list[i]->SetIp(addr);
				player_list[i]->SetName(loginInfo[i].name);
				player_list[i]->SetState(loginInfo[i].state);

				printf("name : %s	state : %d\n", player_list[i]->GetName(), player_list[i]->GetState());
			}
			break;
		}
		case SC_READY:
		{
			char IdBuf[sizeof(short)]{};
			recv(sock, IdBuf, sizeof(IdBuf), 0);

			short id;
			memcpy(&id, &IdBuf, sizeof(short));
			player_list[id]->SetState(READY);
				

			break;
		}
		case SC_ALLREADY:
			framework->ChangeScene(3);
			break;

			
		case SC_PLAY:
		{
			//printf("playing\n");
			for (int i = 0; i < PLAYER_NUM; ++i)
			{
				if (player_list[i])
					player_list[i]->SetState(PLAYING);
			}
			char Buf[2]{};
			recv(sock, Buf, sizeof(Buf), 0);

			char PlayBuf[sizeof(PLAYER_INFO[3])]{};
			recv(sock, PlayBuf, sizeof(PlayBuf), 0);
			char TimeBuf[sizeof(int)]{};
			recv(sock, TimeBuf, sizeof(TimeBuf), 0);
			//printf("packet size :  %d\n", buf[0]);

			PLAYER_INFO p_info[3];
			memcpy(&p_info, &PlayBuf, sizeof(PLAYER_INFO[3]));


			//printf("id : %d %d %d\n", p_info[0].ID, p_info[1].ID, p_info[2].ID)
			//framework->play_scene->Player_Info.time = temp.time;
			framework->play_scene->SetPlayerInfo(p_info);

			break;
		}

		case SC_RESULT: //질문, 우리 1개씩 받아오는건데 배열 크기 3으로 지정한 이유가있나용 
		{
			printf("result\n");
			//char PlayBuf[sizeof(P_STATE[3])]{}; // 잡은 몬스터와 죽은 횟수로 수정 필요
			//recv(sock, PlayBuf, sizeof(PlayBuf), 0);
			printf("packet size :  %d\n", buf[0]);

			char Buf[2]{};
			recv(sock, Buf, sizeof(Buf), 0);

			char ResultBuf[sizeof(P_STATE_INFO)]{};
			recv(sock, ResultBuf, sizeof(ResultBuf), 0);

			P_STATE_INFO p_info;
			memcpy(&p_info, &ResultBuf, sizeof(P_STATE_INFO));
			//if (player_list[0] && p_info->state == 0)/*printf("ID:%d-----\n", p_info[0].dungeonID);*/framework->StartNum = 1; 
			
			if (p_info.dungeonID != framework->dungeonID) { //던전 아이디가 바뀔시 playscene를 전환하기 위해 bool활성 
				framework->StartNum = 1;
				framework->dungeonID = p_info.dungeonID;
				printf("ID:%d----------------------\n", p_info.dungeonID);
			}
			

			for (int i = 0; i < PLAYER_NUM; ++i)
			{
				if (player_list[i] && p_info.state == 0) {
					printf("change to playing\n");
					player_list[i]->SetState(PLAYING);
					framework->ChangeScene(3);
				}
				else if (player_list[i])
					player_list[i]->SetState(RESULTING);

			}
			break;
		}
		
		case SC_MAKE_MONSTER:
		{
			char Buf[2];
			recv(sock, Buf, sizeof(Buf), 0);

			char subBuf[sizeof(MAKE_MONSTER[5])]{};
			recv(sock, subBuf, sizeof(subBuf), 0);

			MAKE_MONSTER monsterInfo[5];
			memcpy(&monsterInfo, &subBuf, sizeof(MAKE_MONSTER[5]));

			framework->GetPlayScene()->GetMonsterManager()->Appear(monsterInfo);
			break;
		}
		case SC_MONSTER:
		{
			char Buf[2];
			recv(sock, Buf, sizeof(Buf), 0); 

			char subBuf[sizeof(MONSTER_INFO_MANAGER[20])]{};
			recv(sock, subBuf, sizeof(subBuf), 0);

			MONSTER_INFO_MANAGER monsterInfo[20];
			memcpy(&monsterInfo, &subBuf, sizeof(MONSTER_INFO_MANAGER[20]));

			framework->GetPlayScene()->GetMonsterManager()->UpdateMonsterInfo(monsterInfo, framework->GetPlayScene()->GetPlayer(), framework->GetPlayScene()->GetMissileManager(), framework->GetPlayScene()->GetAnimationManager());
			break;
		}
		case SC_MISSILE:
		{	
			char Buf[2];
			recv(sock, Buf, sizeof(Buf), 0);
			char subBuf[sizeof(MISSILE_INFO[MISSILE_NUM])];
			recv(sock, subBuf, sizeof(subBuf), 0);

			MISSILE_INFO missile[MISSILE_NUM];
			memcpy(&missile, &subBuf, sizeof(MISSILE_INFO[MISSILE_NUM]));

			framework->GetPlayScene()->UpdateMissile(missile);
			break;
		}
		default:
			printf("Unknown PACKET type [%d]\n", type);//렉떄문에 잠시 주석
			//break;
		}
	}
}
