#include <QLabel>
#include <QMessageBox>
#include <QCoreApplication>
#include "MainWindow.h"
#include "FrameEvent.h"
#include <iostream>
#include <cstring>

namespace Speckle {

int MainWindow::FrameEventType = -1;

MainWindow::MainWindow()
	: m_label(new QLabel(this)),
	m_done(false)
{
	if (FrameEventType == -1) {
		FrameEventType = QEvent::registerEventType();
	}

	m_kinectThread.reset(new std::thread( [=] {
		kinectThreadMain();
	}));
	m_label->setMinimumSize(640, 488);
	resize(640, 488);
}

MainWindow::~MainWindow() {
	m_done = true;
	if (m_kinectThread->joinable()) {
		m_kinectThread->join();
	}
}

void MainWindow::kinectThreadMain() {
	freenect_context *ctx;
	freenect_device *dev;

	if (freenect_init(&ctx, NULL) < 0) {
		fatal("Error: freenect_init() failed");
		return;
	}

	freenect_set_log_level(ctx, FREENECT_LOG_INFO);
	
	freenect_select_subdevices(ctx, FREENECT_DEVICE_CAMERA);
	if (freenect_num_devices(ctx) < 1) {
		fatal("Error: no Kinect devices found");
		return;
	}

	if (freenect_open_device(ctx, &dev, 0) < 0) {
		fatal("Error: could not open device");
		return;
	}

	freenect_frame_mode frameMode = freenect_find_video_mode(
		FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_IR_10BIT_PACKED);

	ComputePipeline::Options options;
	options.width = frameMode.width;
	options.height = frameMode.height;
	options.bitsPerPixel = 10;
	options.frameSize = options.bitsPerPixel * frameMode.width * frameMode.height / 8;
	m_frameBuffer.create(options.frameSize, 1, CV_8UC1);

	m_pipeline.reset(new ComputePipeline(options));

	freenect_set_video_callback(dev, VideoCallback);
	freenect_set_video_mode(dev, frameMode);
	freenect_set_user(dev, (void*)this);
	freenect_set_ir_brightness(dev, 40);
	freenect_start_video(dev);

	while (!m_done && freenect_process_events(ctx) >= 0);

	freenect_stop_video(dev);
	freenect_close_device(dev);
	freenect_shutdown(ctx);
}

void MainWindow::VideoCallback(freenect_device *dev, void *data, uint32_t timestamp) {
	MainWindow & main = *(MainWindow*)freenect_get_user(dev);
	main.processFrame(dev, data, timestamp);
}

void MainWindow::processFrame(freenect_device *dev, void *data, uint32_t timestamp) {
	int bitsPerPixel = 10;
	if (m_inFlight.load()) {
		// Main thread has not displayed the previous frame yet
		return;
	}
	m_inFlight++;

	freenect_frame_mode frameMode = freenect_get_current_video_mode(dev);
	size_t size = bitsPerPixel * frameMode.width * frameMode.height / 8;
	std::memcpy(m_frameBuffer.ptr(0), data, size);

	FrameEvent * event = new FrameEvent(FrameEventType, m_frameBuffer.ptr(0),
			size, frameMode.width, frameMode.height);
	QCoreApplication::postEvent(this, event);
}

void MainWindow::customEvent(QEvent * event) {
	if (event->type() != FrameEventType) {
		return;
	}
	FrameEvent * fe = static_cast<FrameEvent *>(event);

	if (m_mat.empty()) {
		m_mat.create(fe->height, fe->width, CV_8UC4);
	}
	m_pipeline->writeFrame(fe->data, fe->size, m_mat, CV_8UC4);
	QPixmap pixmap = QPixmap::fromImage(QImage(
			m_mat.ptr(0), fe->width, fe->height, QImage::Format_RGB32));
	if (fe->width > 640) {
		pixmap = pixmap.scaled(pixmap.size() / 2);
	}
	m_label->setPixmap(pixmap);
	m_inFlight--;
}


void MainWindow::fatal(const char * message) {
	QMessageBox::critical(nullptr, "Error", message);
}

} // namespace
