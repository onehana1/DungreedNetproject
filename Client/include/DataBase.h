#pragma once
#ifndef _database
#define _database
#include <string>
#include <tuple>
#include <vector>
#include <fstream>
#include <cctype>
#include <tchar.h>
#include <windows.h>
#include "Uncopyable.h"
#include "FileUtility.h"
#include <cassert>

namespace DB {
	constexpr int DEF_STR_LEN = 200;

	template<typename T>
	class Catching_Return		// �������� ���� ���� Ÿ��, ������ �����ϸ� ���� Ÿ�� ��� return_success = false �� ��ȯ�Ѵ�
	{
	public:
		T inst;
		bool return_success = true;
		Catching_Return(T inst) : inst{ inst } {}
		Catching_Return(bool return_success) : return_success{ return_success } {}
		operator T() { return inst; }
		operator bool() { return return_success; }
	};

	Catching_Return<POINT> GetPointFromStr(const std::string& str);
	Catching_Return<int> GetIntFromStr(const std::string& str);

	// ------------------------------------------------- generic programming���� ������ �κ� -----------
	// �������� ���ø����� �����ͺ��̽� Ŭ������ �����ϵ��� �Ѵ�.
	// �����ͺ��̽��� ���ø� ���ڴ� �����ͺ��̽��� ���� �ڷ����̴�.
	// �����ͺ��̽��� �������� ���ڷ� []<typename T>(args){ return T(my_args); } �� �޴´�.
	// 	   �̶� args�� �⺻ �ڷ����� ���ڿ��� �Ѵ�. my_args�� args�� �״�� ���ų� T �����ڿ� ���̴� ���ڷ� �����ϸ� �ȴ�.
	// �����ͺ��̽��� �� Ʃ���� 0.�ʵ��� type_info 1. �ʵ��� �ּ� 2. �ε� ���η� �����ǵ��� �Ѵ�.
	// �ε�� field_type_info.name() == typeid(T).name()�� �� *static_cast<T*>(tuple_addr) = []<typename T>(args); �� �����Ѵ�.
	// -----------------------------------------------------------------------------------------------

	interface DB_Data
	{
	public:
		virtual ~DB_Data() {}
	};

	class DB_Point : public DB_Data
	{
	public:
		POINT data;

		DB_Point(const int x, const int y) : data{ POINT{ x, y } } {}
		DB_Point(POINT pt) : data{ pt } {}
	};

	class DB_String : public DB_Data
	{
	public:
		TCHAR data[DEF_STR_LEN];

		DB_String(const TCHAR* str) { lstrcpy(data, str); }
	};

	class DB_Int : public DB_Data
	{
	public:
		int data;

		DB_Int(const int val) : data{ val } {}
	};


	typedef std::shared_ptr<DB_Data> Data_Inst;

	// -----------------------------------------------------------------------------------------------




	// Ʃ�� 0. �ʵ� �̸� 1. �ʵ� �ּ� 2. �ε� ����
	typedef std::vector<std::tuple<std::string, void*, bool>> DataKit;

	class IDConfig : private Uncopyable
	{
	private:
		const int ID_DIGIT = 7;
		std::ifstream* in;
		std::string line;
		int cur_id = 0;

		IDConfig(const IDConfig& other) {}
		IDConfig& operator=(const IDConfig& other) {}

	public:
		IDConfig() = default;

		inline void AllocateFstream(std::ifstream* in) { this->in = in; }
		Catching_Return<int> Acc2ID();
		Catching_Return<int> Acc2ID(const int id);
		Catching_Return<int> GetNextID();
		bool IsID(const std::string& str) const;
		inline int GetCurID() const { return cur_id; }
	};

	class DataBase : private Uncopyable
	{
	private:
		DataKit data_kit;
		IDConfig id_config;

		std::ifstream in;
		std::string line;

		void LoadIndividual();

		void InterpretLine();

		Catching_Return<void*> GetFieldInst(std::string field_str);
		const Data_Inst GetDataInst(std::string data_str) const;
		void Match(void* addr, const Data_Inst& inst);

		void CompleteCheck() const;

	public:
		DataBase(const TCHAR* db_name) : in{ db_name } { id_config.AllocateFstream(&in); }

		void RegisterField(const std::string& field_name, void* const field_addr);

		void Init();
		void Load();
		void Load(const int id);
	};

}

#endif