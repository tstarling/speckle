#ifndef SPECKLE_MAINWINDOW_H
#define SPECKLE_MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <memory>
#include <atomic>
#include <libfreenect.h>
#include <opencv2/core/core.hpp>
#include "compute/ComputePipeline.h"

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

namespace Speckle {

class MainWindow : public QMainWindow {
public:
	MainWindow();
	~MainWindow();
	virtual void customEvent(QEvent * event);

private:
	void kinectThreadMain();
	static void VideoCallback(freenect_device *dev, void *data, uint32_t timestamp);
	void processFrame(freenect_device *dev, void *data, uint32_t timestamp);
	void fatal(const char * message);

	QLabel * m_label;

	cv::Mat m_frameBuffer;
	cv::Mat m_mat;
	int m_input;
	int m_display;

	std::unique_ptr<std::thread> m_kinectThread;
	std::unique_ptr<ComputePipeline> m_pipeline;
	bool m_done;
	std::atomic<int> m_inFlight;

	static int FrameEventType;
};

} // namespace
#endif
