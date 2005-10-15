#ifndef __PSPEventQ__
	#define __PSPEventQ__

	#include <list>
	using namespace std;
	
	class CPSPEventQ
	{
	public:
		struct QEvent
		{
			u32 SenderId;
			u32 EventId;
			void *pData;
		};
		
	public:
		CPSPEventQ(char *strName);
		~CPSPEventQ();
		int Send(QEvent &Event);
		int SendAndWaitForOK(QEvent &Event);
		int Receive(QEvent &Event);
		int SendReceiveOK();
		int Size();
		void Clear();
		
	private:
		char *m_strName;
		list<QEvent> m_EventList; 
		CBlocker *m_RcvBlocker, *m_RcvOKBlocker;
		CLock *m_lock;

};

#endif
