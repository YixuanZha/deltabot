from rknlite.api import RKNNLite

rknn_lite = RKNNLite()
rknn_lite.load_rknn('./yolov8n-rk3588.rknn')
rknn_lite.init_runtime()

outputs = rknn_lite.inference(inputs=[input_data])
rknn_lite.release()
