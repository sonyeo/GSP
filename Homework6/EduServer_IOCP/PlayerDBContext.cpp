#include "stdafx.h"
#include "SQLStatement.h"
#include "Log.h"
#include "PlayerDBContext.h"
#include "DBHelper.h"
#include "ClientSession.h"


//todo: CreatePlayerDataContext 구현

//todo: DeletePlayerDataContext 구현



// param으로 넣어야 할건, 해당 Context의 생성자의 인자로 넣어놓은걸 쓰고
// 결과로 받아야 할건, 해당 Context의 멤버변수로 받아놓음
bool LoadPlayerDataContext::OnSQLExecute()
{
	// 이걸 로컬 변수로 생성하면,
	// 생성자에서 해당 thread의 thread local storage에 보관된 STMT 핸들을 DBHelper의 멤버변수에 대입해놓음
	DBHelper dbHelper;

	// param 추가
	dbHelper.BindParamInt(&mPlayerId);

	// 결과 추가
	dbHelper.BindResultColumnText(mPlayerName, MAX_NAME_LEN);
	dbHelper.BindResultColumnFloat(&mPosX);
	dbHelper.BindResultColumnFloat(&mPosY);
	dbHelper.BindResultColumnFloat(&mPosZ);
	dbHelper.BindResultColumnBool(&mIsValid);
	dbHelper.BindResultColumnText(mComment, MAX_COMMENT_LEN);

	// Half - sync Half - async
	// DB 작업은 SYNC하게 하고, PostDatabaseResult()로 IOCP에 넣어주는 방식으로 하면 될 듯 한데..
	if (dbHelper.Execute(SQL_LoadPlayer))
	{
		if (dbHelper.FetchRow())
		{
			return true;
		}
	}

	return false;
}

void LoadPlayerDataContext::OnSuccess()
{
	//todo: 플레이어 로드 성공시 처리하기
	
}

void LoadPlayerDataContext::OnFail()
{
	EVENT_LOG("LoadPlayerDataContext fail", mPlayerId);
}


bool UpdatePlayerPositionContext::OnSQLExecute()
{
	DBHelper dbHelper;
	int result = 0;

	dbHelper.BindParamInt(&mPlayerId);
	dbHelper.BindParamFloat(&mPosX);
	dbHelper.BindParamFloat(&mPosY);
	dbHelper.BindParamFloat(&mPosZ);

	dbHelper.BindResultColumnInt(&result);

	if (dbHelper.Execute(SQL_UpdatePlayerPosition))
	{
		if (dbHelper.FetchRow())
		{
			return result != 0;
		}
	}

	return false;
}

// 성공시에는 Context의 멤버변수들을 인자로, 정해진 callback함수를 호출함
void UpdatePlayerPositionContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseUpdatePosition(mPosX, mPosY, mPosZ);
}




bool UpdatePlayerCommentContext::OnSQLExecute()
{
	DBHelper dbHelper;
	int result = 0;
	dbHelper.BindParamInt(&mPlayerId);
	dbHelper.BindParamText(mComment);

	dbHelper.BindResultColumnInt(&result);

	if (dbHelper.Execute(SQL_UpdatePlayerComment))
	{
		if (dbHelper.FetchRow())
		{
			return result != 0;
		}
	}

	return false;
}

void UpdatePlayerCommentContext::SetNewComment(const wchar_t* comment)
{
	wcscpy_s(mComment, comment);
}

void UpdatePlayerCommentContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseUpdateComment(mComment);
}



bool UpdatePlayerValidContext::OnSQLExecute()
{
	DBHelper dbHelper;
	int result = 0;

	dbHelper.BindParamInt(&mPlayerId);
	dbHelper.BindParamBool(&mIsValid);

	dbHelper.BindResultColumnInt(&result);

	if (dbHelper.Execute(SQL_UpdatePlayerValid))
	{
		if (dbHelper.FetchRow())
		{
			return result !=0 ;
		}
	}

	return false;
}

void UpdatePlayerValidContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseUpdateValidation(mIsValid);
}


