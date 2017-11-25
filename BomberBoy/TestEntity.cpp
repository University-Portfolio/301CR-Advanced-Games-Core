#include "TestEntity.h"
CLASS_SOURCE(ATestEntity)

#include "Core\Level.h"
#include "Core\Game.h"
#include "Core\PlayerController.h"

ATestEntity::ATestEntity()
{
	bIsNetSynced = true;
	bIsTickable = true;
}

bool ATestEntity::RegisterRPCs(const char* func, RPCInfo& outInfo) const
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(TCP, RPCCallingMode::Host, ResetPosition);
	RPC_INDEX(UDP, RPCCallingMode::Broadcast, MoveTo);
	RPC_INDEX(UDP, RPCCallingMode::Broadcast, PrintTime);
	return false;
}

bool ATestEntity::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC(ResetPosition);
	RPC_EXEC_TwoParam(MoveTo, float, float);
	RPC_EXEC_TwoParam(PrintTime, float, float);
	return false;
}


void ATestEntity::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode)
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode)
	SYNCVAR_INDEX(TCP, SyncVarMode::OnChange, int, clampedTimer)
	SYNCVAR_INDEX(TCP, SyncVarMode::OnChange, string, testString)
}

bool ATestEntity::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks)
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC_Callback(clampedTimer, OnClampedTimerChange);
	SYNCVAR_EXEC_Callback(testString, OnTestStringChange);
	return false;
}

void ATestEntity::OnBegin()
{
	Super::OnBegin();
	startPos = GetLocation();
	timer = 0;

	m_testAnimation.SetFrameDuration(0.15f);
	m_testAnimation.AddFrame(GetAssetController()->GetTexture("Resources\\Character\\Down_0.png"));
	m_testAnimation.AddFrame(GetAssetController()->GetTexture("Resources\\Character\\Down_1.png"));
	m_testAnimation.AddFrame(GetAssetController()->GetTexture("Resources\\Character\\Down_0.png"));
	m_testAnimation.AddFrame(GetAssetController()->GetTexture("Resources\\Character\\Down_2.png"));


	//if (GetGame()->GetSession() == nullptr)
	//{
	//	GetGame()->GetEngine()->GetNetController()->JoinSession(GetGame()->GetEngine()->GetDefaultNetIdentity());
	//}
}

void ATestEntity::OnDestroy() 
{
	Super::OnDestroy();
	LOG("Is Ded");
}

void ATestEntity::OnTick(const float& deltaTime)
{
#ifdef BUILD_SERVER
	/*
	timer += deltaTime; 
	if (timer < 2)
		return;
	timer = 0;
	clampedTimer++;
	//testString += "(len:" + std::to_string(testString.size()) + ")";

	//CallRPC_TwoParam(this, MoveTo, timer * 20.0f, 0);

	//if (timer < 10.0f)
	//	return;
	//Destroy(this);
	*/
#else
#endif

}

#ifdef BUILD_CLIENT
void ATestEntity::OnDraw(sf::RenderWindow* window, const float& deltaTime)
{
	m_testAnimation.UpdateAnimation(deltaTime);

	sf::RectangleShape rect;
	rect.setPosition(GetLocation());
	rect.setSize(vec2(16, 21) * 2.0f);
	rect.setTexture(m_testAnimation.GetCurrentFrame());

	rect.setOutlineThickness(1);
	rect.setOutlineColor(sf::Color(0, 255, 255));
	window->draw(rect);
}
#endif

void ATestEntity::PrintTime(float time, float time2)
{
	LOG("Time, %f, %f", time, time2);
	//SetLocation(startPos + vec2(time, -time) * 20.0f);
}

void ATestEntity::ResetPosition()
{
	SetLocation(startPos);
}

void ATestEntity::MoveTo(float x, float y)
{
	SetLocation(startPos + sf::Vector2f(x, y));
}


void ATestEntity::OnClampedTimerChange()
{
	LOG("Called OnClampedTimerChange %i", clampedTimer);
}

void ATestEntity::OnTestStringChange()
{
	LOG("Called OnTestStringChange:\n%s", testString.c_str());
}
