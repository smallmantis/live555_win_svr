#include "H264LiveVideoServerMediaSubsession.h"

H264LiveVideoServerMediaSubsession::H264LiveVideoServerMediaSubsession(UsageEnvironment & env, FramedSource * source) : OnDemandServerMediaSubsession(env, True)
{
	m_pSource = source;
	m_pSDPLine = 0;
}

H264LiveVideoServerMediaSubsession::~H264LiveVideoServerMediaSubsession(void)
{
	if (m_pSDPLine)
	{
		free(m_pSDPLine);
	}
}

H264LiveVideoServerMediaSubsession * H264LiveVideoServerMediaSubsession::createNew(UsageEnvironment & env, FramedSource * source)
{
	return new H264LiveVideoServerMediaSubsession(env, source);
}

FramedSource * H264LiveVideoServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned & estBitrate)
{
	return H264VideoStreamFramer::createNew(envir(), new H264LiveVideoSource(envir()));
}

RTPSink * H264LiveVideoServerMediaSubsession::createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource)
{
	return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

char const * H264LiveVideoServerMediaSubsession::getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource)
{
	if (m_pSDPLine)
	{
		return m_pSDPLine;
	}

	m_pDummyRTPSink = rtpSink;

	//mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);
	m_pDummyRTPSink->startPlaying(*inputSource, 0, 0);

	chkForAuxSDPLine(this);

	m_done = 0;

	envir().taskScheduler().doEventLoop(&m_done);

	m_pSDPLine = _strdup(m_pDummyRTPSink->auxSDPLine());

	m_pDummyRTPSink->stopPlaying();

	return m_pSDPLine;
}

void H264LiveVideoServerMediaSubsession::afterPlayingDummy(void * ptr)
{
	H264LiveVideoServerMediaSubsession * This = (H264LiveVideoServerMediaSubsession *)ptr;

	This->m_done = 0xff;
}

void H264LiveVideoServerMediaSubsession::chkForAuxSDPLine(void * ptr)
{
	H264LiveVideoServerMediaSubsession * This = (H264LiveVideoServerMediaSubsession *)ptr;

	This->chkForAuxSDPLine1();
}

void H264LiveVideoServerMediaSubsession::chkForAuxSDPLine1()
{
	if (m_pDummyRTPSink->auxSDPLine())
	{
		m_done = 0xff;
	}
	else
	{
		double delay = 1000.0 / (FRAME_PER_SEC);  // ms
		int to_delay = delay * 1000;  // us

		nextTask() = envir().taskScheduler().scheduleDelayedTask(to_delay, chkForAuxSDPLine, this);
	}
}