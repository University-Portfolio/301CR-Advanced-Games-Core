/**
* Used macros to provide a nice API approach to adding support of RPCs and synced vars
*/
#pragma once
#include "Common.h"
#include "Encoding.h"
#include "ByteBuffer.h"
#include "NetSocket.h"


/**
* The different calling modes avaliable when calling an RPC
*/
enum class RPCCallingMode : uint8 
{
	Unknown		= 0,		
	Host		= 1,	// Execute RPC on host
	Owner		= 2,	// Execute RPC on owning client
	Broadcast	= 3,	// Execute RPC on all clients (Only valid when called by server)
};

/**
* Describes a registered RPC and how to call it
*/
struct RPCInfo 
{
	uint16			index;			// Registered index of this RPC
	RPCCallingMode	callingMode;	// Calling mode of this RPC
	SocketType		socket;			// What socket the RPC call should be sent over
};


/**
* Describes a call request for an RPC
*/
struct RPCRequest 
{
	RPCInfo		function;	// Registered RPC information
	ByteBuffer	params;		// Params to call the function using
};
typedef std::vector<RPCRequest> RPCQueue;

template<>
inline void Encode<RPCRequest>(ByteBuffer& buffer, const RPCRequest& data)
{
	Encode<uint16>(buffer, data.function.index);
	Encode<uint8>(buffer, (uint8)data.function.callingMode);
	Encode<uint16>(buffer, data.params.Size());
	buffer.Push(data.params.Data(), data.params.Size());
}

template<>
inline bool Decode<RPCRequest>(ByteBuffer& buffer, RPCRequest& out, void* context)
{
	uint8 target;
	uint16 paramCount;

	if (!Decode<uint16>(buffer, out.function.index) ||
		!Decode<uint8>(buffer, target) ||
		!Decode<uint16>(buffer, paramCount)
		)
		return false;

	out.function.callingMode = (RPCCallingMode)target;
	buffer.PopBuffer(out.params, paramCount);
	return true;
}



/**
* All the different roles a net synced object can have
*/
enum class NetRole : uint8
{
	None = 0,
	RemotePuppet,			// Doesn't have any control over this object whatsoever
	HostPuppet,				// Doesn't own this object, but is the host, so can edit
	RemoteOwner,			// Owns the object, but is not the host
	HostOwner,				// Owns the object and is the host
};



class NetSession;



/**
* Lets child classes register RPCs and synced variables
* O-------------------------O
* RPCs (For best performance they shouldn't return anything):
* Override both FetchRPCIndex and ExecuteRPC.
* -FetchRPCIndex: place RPC_INDEX_HEADER at top passing passed arguments and use RPC_INDEX for each function you want to register
* -ExecuteRPC: place RPC_EXEC_HEADER at top passing passed arguments and use RPC_EXEC for each function you want to register (In the same order as in RPC_INDEX)
* --If you want to call a function that has arguments use RPC_EXEC_OneParam, RPC_EXEC_TwoParam etc. up until Five
* --If you want more that 5 arguments use a struct and add an Encode and Decode template implementation
* O-------------------------O
*/
class CORE_API NetSerializableBase
{
private:
	friend NetSession;

	NetRole m_netRole = NetRole::None;
	uint16 m_networkOwnerId = 0;
	uint16 m_networkId = 0;
	bool bFirstNetUpdate;

	RPCQueue m_UdpRpcQueue;
	RPCQueue m_TcpRpcQueue;

protected:
	bool bIsNetSynced = false;

public:
	/**
	* Update this object's role, based on the current session information
	* @param session			The session which is currently active
	* @param assignOwner		Should this object assume the session as it's owner
	*/
	void UpdateRole(const NetSession* session, const bool& assignOwner = false);


	/**
	* RPC Override pair
	*/
public:
	/**
	* Register RPCs in this function
	* @param func			The name of the function
	* @param outInfo		Information about the RPC
	* @returns If the function is registered or not
	*/
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const;
protected:
	/**
	* Call a given function from it's RPC id
	* NOTE: macro order between RegisterRPCs and ExecuteRPC must align
	* @param id				The RPC id of the function
	* @param params			The raw parameters for the function to use
	* @returns If call succeeds
	*/
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params);
public:

	/**
	* Enqueue an RPC to be executed by the server, client
	* (If no session is active, they will just execute normally)
	* @param rpcInfo		The RPC to call
	* @param params			Encoded parameters to call the function using
	*/
	void RemoteCallRPC(const RPCInfo& rpcInfo, const ByteBuffer& params);



	/**
	* Has this object got any data to encode
	* @param socketType			The socket type the data will be sent over
	* @returns True if there is pending data
	*/
	inline bool HasQueuedNetData(const SocketType& socketType) const
	{
		if (socketType == TCP)
			return m_TcpRpcQueue.size() != 0;
		else
			return m_UdpRpcQueue.size() != 0;
	}
	/**
	* Clears any net data which is currently queued
	*/
	inline void ClearQueuedNetData()
	{
		m_UdpRpcQueue.clear();
		m_TcpRpcQueue.clear();
	}



private:
	/**
	* Encode all currently queued RPC calls
	* @param targetNetId	The net id of where this data will be sent to
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this will be sent over
	*/
	void EncodeRPCRequests(const uint16& targetNetId, ByteBuffer& buffer, const SocketType& socketType);
	/**
	* Decode all RPC calls in this queue
	* @param sourceNetId	The net id of where this data came from
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this was sent over
	*/
	void DecodeRPCRequests(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType);
	

	/**
	* Getters & Setters
	*/
public:
	inline const bool& IsNetSynced() const { return bIsNetSynced; }

	/**
	* A unique ID for identifying this object over the current NetSession
	*/
	inline const uint16& GetNetworkID() const { return m_networkId; }
	inline const uint16& GetNetworkOwnerID() const { return m_networkOwnerId; }

	inline const NetRole& GetNetRole() const { return m_netRole; }
	inline const bool IsNetOwner() const { return m_netRole == NetRole::None || m_netRole == NetRole::HostOwner || m_netRole == NetRole::RemoteOwner; }
	inline const bool IsNetHost() const { return m_netRole == NetRole::HostOwner || m_netRole == NetRole::HostPuppet; }
	inline const bool HasNetControl() const { return IsNetOwner() || IsNetHost(); }
};



/**
* Placed at the start of FetchRPCIndex to create temporary vars (To avoid naming problems)
* and to handle parent calls correctly
*/
#define RPC_INDEX_HEADER(func, outInfo) \
	const char*& __TEMP_NAME = func; \
	RPCInfo& __TEMP_INFO = outInfo; \
	if(__super::RegisterRPCs(__TEMP_NAME, __TEMP_INFO)) return true; 

/**
* Placed after RPC_INDEX_HEADER in FetchRPCIndex to create an entry for a function
*/
#define RPC_INDEX(socketType, mode, func) \
	if (std::strcmp(__TEMP_NAME, #func) == 0) \
	{ \
		__TEMP_INFO.callingMode = mode; \
		__TEMP_INFO.socket = socketType; \
		return true; \
	} \
	else \
		++__TEMP_INFO.index;




/**
* Placed at the start of ExecuteRPC to create temporary vars (To avoid naming problems)
* and to handle parent calls correctly
*/
#define RPC_EXEC_HEADER(id, params) \
	uint16 __TEMP_ID = id; \
	ByteBuffer& __TEMP_BUFFER = params; \
	if(__super::ExecuteRPC(__TEMP_ID, __TEMP_BUFFER)) return true;


/**
* Execution for a function at the placed index
* Containing 0 parameters
*/
#define RPC_EXEC(func) \
	if (__TEMP_ID == 0) \
	{ \
		func(); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 1 parameters
*/
#define RPC_EXEC_OneParam(func, paramAType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		if(Decode<paramAType>(__TEMP_BUFFER, A)) \
			func(A); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 2 parameters
*/
#define RPC_EXEC_TwoParam(func, paramAType, paramBType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B)) \
			func(A, B); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 3 parameters
*/
#define RPC_EXEC_ThreeParam(func, paramAType, paramBType, paramCType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		paramCType C; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B) && \
			Decode<paramCType>(__TEMP_BUFFER, C)) \
			func(A, B, C); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 4 parameters
*/
#define RPC_EXEC_FourParam(func, paramAType, paramBType, paramCType, paramDType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		paramCType C; \
		paramDType D; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B) && \
			Decode<paramCType>(__TEMP_BUFFER, C) && \
			Decode<paramDType>(__TEMP_BUFFER, D)) \
			func(A, B, C, D); \
		return true; \
	} \
	else \
		--__TEMP_ID;

/**
* Execution for a function at the placed index
* Containing 4 parameters
*/
#define RPC_EXEC_FiveParam(func, paramAType, paramBType, paramCType, paramDType, paramEType) \
	if (__TEMP_ID == 0) \
	{ \
		paramAType A; \
		paramBType B; \
		paramCType C; \
		paramDType D; \
		paramEType E; \
		if(	Decode<paramAType>(__TEMP_BUFFER, A) && \
			Decode<paramBType>(__TEMP_BUFFER, B) && \
			Decode<paramCType>(__TEMP_BUFFER, C) && \
			Decode<paramDType>(__TEMP_BUFFER, D) && \
			Decode<paramEType>(__TEMP_BUFFER, E)) \
			func(A, B, C, D, E); \
		return true; \
	} \
	else \
		--__TEMP_ID;



#define __CallRPC(object, func, funcCall, funcEncode) \
{ \
	NetSerializableBase* __TEMP_NSB = static_cast<NetSerializableBase*>(object); \
	RPCInfo __TEMP_INFO; \
	if (__TEMP_NSB->RegisterRPCs(#func, __TEMP_INFO)) \
	{ \
		if(GetNetworkID() != 0) \
		{ \
			if ((__TEMP_NSB->IsNetHost() && __TEMP_INFO.callingMode == RPCCallingMode::Host) || (__TEMP_NSB->IsNetOwner() && __TEMP_INFO.callingMode == RPCCallingMode::Owner)) \
				object->funcCall; \
			else if (__TEMP_NSB->IsNetHost() || __TEMP_INFO.callingMode != RPCCallingMode::Broadcast) \
			{ \
				ByteBuffer __TEMP_BUFFER; \
				funcEncode \
				__TEMP_NSB->RemoteCallRPC(__TEMP_INFO, __TEMP_BUFFER); \
			} \
			else \
				LOG_ERROR("Invalid rights to call function '" #func "'"); \
		} \
	} \
	else \
		LOG_ERROR("Cannot call function '" #func "' as it is not a registered RPC for the given object"); \
}

/**
* Execute RPC with given settings
* Containing 0 parameters
*/
#define CallRPC(object, func) __CallRPC(object, func, func(),)

/**
* Execute RPC with given settings
* Containing 1 parameters
*/
#define CallRPC_OneParam(object, func, paramA) __CallRPC(object, func, func(paramA), Encode(__TEMP_BUFFER,paramA);)

/**
* Execute RPC with given settings
* Containing 2 parameters
*/
#define CallRPC_TwoParam(object, func, paramA, paramB) __CallRPC(object, func, func(paramA, paramB), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);)

/**
* Execute RPC with given settings
* Containing 3 parameters
*/
#define CallRPC_ThreeParam(object, func, paramA, paramB, paramC) __CallRPC(object, func, func(paramA, paramB, paramC), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);Encode(__TEMP_BUFFER,paramC);)

/**
* Execute RPC with given settings
* Containing 4 parameters
*/
#define CallRPC_FourParam(object, func, paramA, paramB, paramC, paramD) __CallRPC(object, func, func(paramA, paramB, paramC, paramD), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);Encode(__TEMP_BUFFER,paramC);Encode(__TEMP_BUFFER,paramD);)

/**
* Execute RPC with given settings
* Containing 5 parameters
*/
#define CallRPC_FiveParam(object, func, paramA, paramB, paramC, paramD, paramE) __CallRPC(object, func, func(paramA, paramB, paramC, paramD, paramE), Encode(__TEMP_BUFFER,paramA);Encode(__TEMP_BUFFER,paramB);Encode(__TEMP_BUFFER,paramC);Encode(__TEMP_BUFFER,paramD);Encode(__TEMP_BUFFER,paramE);)