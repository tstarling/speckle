#ifndef SPECKLE_FRAMEEVENT_H
#define SPECKLE_FRAMEEVENT_H

#include <QEvent>
#include <QImage>

namespace Speckle {

class FrameEvent : public QEvent {
public:
	FrameEvent(int type, void * data, size_t size, int width, int height)
		: QEvent((QEvent::Type)type), data(data), size(size), width(width), height(height)
	{}

	void * data;
	size_t size;
	int width;
	int height;
};

} // namespace

#endif
