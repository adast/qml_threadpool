all: build_container run

build_container:
	docker build -t qml_threadpool .

run:
	docker run --rm -it \
		--name qml_threadpool \
		-v $XAUTHORITY:/root/.Xauthority \
		-v /tmp/.X11-unix:/tmp/.X11-unix \
		-e QT_QUICK_BACKEND=software \
		-e DISPLAY qml_threadpool