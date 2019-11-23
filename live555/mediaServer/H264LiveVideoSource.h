#ifndef _H264LiveVideoSource_H_
#define _H264LiveVideoSource_H_

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "FramedSource.hh"

#define FRAME_PER_SEC 25
#define FMAX (300000)

class H264LiveVideoSource : public FramedSource
{
public:
	H264LiveVideoSource(UsageEnvironment & env);
	~H264LiveVideoSource(void);

public:
	virtual void doGetNextFrame();
	virtual unsigned int maxFrameSize() const;

	static void getNextFrame(void * ptr);
	void GetFrameData();

private:
	void *m_pToken;
	char *m_pFrameBuffer;
};

#endif