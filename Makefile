# SOURCES = cannyedgedetect facedetector linedetection motiondetection opticalflow
SOURCES = cannyedgedetect smiledetect

CXX_FLAGS += \
		-I$(OPENCV_DIR). \
		-I$(OPENCV_DIR)release \
		-I$(OPENCV_DIR)include \
		-I$(OPENCV_DIR)include/opencv \
		-I$(OPENCV_DIR)modules/core/include \
		-I$(OPENCV_DIR)modules/flann/include \
		-I$(OPENCV_DIR)modules/imgproc/include \
		-I$(OPENCV_DIR)modules/video/include \
		-I$(OPENCV_DIR)modules/highgui/include \
		-I$(OPENCV_DIR)modules/ml/include \
		-I$(OPENCV_DIR)modules/calib3d/include \
		-I$(OPENCV_DIR)modules/features2d/include \
		-I$(OPENCV_DIR)modules/objdetect/include \
		-I$(OPENCV_DIR)modules/legacy/include \
		-I$(OPENCV_DIR)modules/contrib/include

# LXX_FLAGS += \
#		$(OPENCV_DIR)release/lib/libopencv_core.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_flann.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_imgproc.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_highgui.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_ml.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_video.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_objdetect.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_features2d.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_calib3d.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_legacy.so.2.3.0 \
#		$(OPENCV_DIR)release/lib/libopencv_contrib.so.2.3.0 \
#		$(OPENCV_DIR)release/lib 

#all: cannyedgedetect.cpp facedetector.cpp framework.cpp linedetection.cpp motiondetection.cpp opticalflow.cpp
#	g++ $(CXX_FLAGS) -o example.o -c example.cpp
#	g++ example.o -o example $(LXX_FLAGS)

all: $(SOURCES)

%.o: %.cpp

clean:
	rm *.o
	rm $(SOURCES)

