#ifndef __PSPMSGQ__
	#define __PSPMSGQ__

	#include <list>
	using namespace std;
	
	class CPSPMessageQ
	{
	public:
		struct QMessage
		{
			u32 SenderId;
			u32 MessageId;
			void *pData;
		};
		
	public:
		CPSPMessageQ(char *strName);
		~CPSPMessageQ();
		int Send(QMessage &Message);
		int SendAndWaitForOK(QMessage &Message);
		int Receive(QMessage &Message);
		int SendReceiveOK();
		int Size();
		void Clear();
		
	private:
		char *m_strName;
		list<QMessage> m_msglist; 
		u32 m_RcvBlockerEventId, m_RcvOKEventId;
		CLock *m_lock;

};

#endif
