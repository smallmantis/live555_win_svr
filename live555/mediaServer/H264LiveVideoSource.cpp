#include "H264LiveVideoSource.h"
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#endif
extern int g_sock;

#define REV_BUF_SIZE  (1024*1024*10)

#ifdef WIN32
#define mSleep(ms)    Sleep(ms)
#else
#define mSleep(ms)    usleep(ms*1000)
#endif


H264LiveVideoSource::H264LiveVideoSource(UsageEnvironment & env) :
FramedSource(env),
m_pToken(0),
m_pFrameBuffer(0)
{
	m_pFrameBuffer = new char[REV_BUF_SIZE];
	if (m_pFrameBuffer == NULL)
	{
		printf("[MEDIA SERVER] error malloc data buffer failed\n");
		return;
	}
	memset(m_pFrameBuffer, 0, REV_BUF_SIZE);
}

H264LiveVideoSource::~H264LiveVideoSource(void)
{
	envir().taskScheduler().unscheduleDelayedTask(m_pToken);

	if (m_pFrameBuffer)
	{
		delete[] m_pFrameBuffer;
		m_pFrameBuffer = NULL;
	}

	printf("[MEDIA SERVER] rtsp connection closed\n");
}

void H264LiveVideoSource::doGetNextFrame()
{
	// 根据 fps，计算等待时间
	double delay = 1000.0 / (FRAME_PER_SEC*2);  // ms
	int to_delay = delay * 1000;  // us

	m_pToken = envir().taskScheduler().scheduleDelayedTask(to_delay, getNextFrame, this);
}

unsigned int H264LiveVideoSource::maxFrameSize() const
{
	return FMAX;
}

void H264LiveVideoSource::getNextFrame(void * ptr)
{
	((H264LiveVideoSource *)ptr)->GetFrameData();
}

void H264LiveVideoSource::GetFrameData()
{
	gettimeofday(&fPresentationTime, 0);

	fFrameSize = 0;
	int len = 0;
	memset(m_pFrameBuffer, 0, REV_BUF_SIZE);
	len = recv(g_sock, m_pFrameBuffer, REV_BUF_SIZE, 0);
	if (len <= 0)
		return;

	fFrameSize = len;
	m_pFrameBuffer[len] = '\0';
	//printf("[MEDIA SERVER] GetFrameData len = [%d],fMaxSize = [%d]\n",fFrameSize,fMaxSize);

	if (fFrameSize > fMaxSize)
	{
		memcpy(fTo, m_pFrameBuffer, fMaxSize);

		fNumTruncatedBytes = fFrameSize - fMaxSize;
		fFrameSize = fMaxSize;
	}
	else
	{
		memcpy(fTo, m_pFrameBuffer, fFrameSize);
		fNumTruncatedBytes = 0;
	}

	afterGetting(this);
}