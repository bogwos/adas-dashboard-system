from tensorflow import keras
import numpy as np
import serial
import time
import cv2

esp = serial.Serial("COM4", 115200)
time.sleep(2)

threshold = 0.79
font = cv2.FONT_HERSHEY_SIMPLEX
model = keras.models.load_model("traffic_sign_model.h5", compile=False)


def preprocess_img(imgBGR, erode_dilate=True):
    rows, cols, _ = imgBGR.shape
    imgHSV = cv2.cvtColor(imgBGR, cv2.COLOR_BGR2HSV)
    Bmin = np.array([100, 43, 46])
    Bmax = np.array([124, 255, 255])
    img_Bbin = cv2.inRange(imgHSV, Bmin, Bmax)

    Rmin1 = np.array([0, 43, 46])
    Rmax1 = np.array([10, 255, 255])
    img_Rbin1 = cv2.inRange(imgHSV, Rmin1, Rmax1)

    Rmin2 = np.array([156, 43, 46])
    Rmax2 = np.array([180, 255, 255])
    img_Rbin2 = cv2.inRange(imgHSV, Rmin2, Rmax2)
    img_Rbin = np.maximum(img_Rbin1, img_Rbin2)
    img_bin = np.maximum(img_Bbin, img_Rbin)

    if erode_dilate:
        kernelErosion = np.ones((3, 3), np.uint8)
        kernelDilation = np.ones((3, 3), np.uint8)
        img_bin = cv2.erode(img_bin, kernelErosion, iterations=2)
        img_bin = cv2.dilate(img_bin, kernelDilation, iterations=2)

    return img_bin


def contour_detect(img_bin, min_area, max_area=-1, wh_ratio=2.0):
    rects = []
    contours, _ = cv2.findContours(
        img_bin.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE
    )

    if len(contours) == 0:
        return rects

    max_area = img_bin.shape[0] * img_bin.shape[1] if max_area < 0 else max_area
    for contour in contours:
        area = cv2.contourArea(contour)
        if min_area <= area <= max_area:
            x, y, w, h = cv2.boundingRect(contour)
            if 1.0 * w / h < wh_ratio and 1.0 * h / w < wh_ratio:
                rects.append([x, y, w, h])

    return rects


def grayscale(img):
    return cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)


def equalize(img):
    return cv2.equalizeHist(img)


def preprocessing(img):
    img = grayscale(img)
    img = equalize(img)
    img = img / 255
    return img


def get_class_name(classNo):
    if classNo == 0:
        return "Speed Limit 20 km/h", 20

    elif classNo == 1:
        return "Speed Limit 30 km/h", 30

    elif classNo == 2:
        return "Speed Limit 50 km/h", 50

    elif classNo == 3:
        return "Speed Limit 60 km/h", 60

    elif classNo == 4:
        return "Speed Limit 70 km/h", 70

    elif classNo == 5:
        return "Speed Limit 80 km/h", 80

    elif classNo == 7:
        return "Speed Limit 100 km/h", 100

    elif classNo == 8:
        return "Speed Limit 120 km/h", 120

    else:
        return None, None


if __name__ == "__main__":
    max_speed = 10
    frame_count = 0

    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    cols = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    rows = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    while True:
        esp.write(bytes([max_speed]))
        print(f"Sent {max_speed}")

        ret, img = cap.read()
        frame_count += 1
        if frame_count % 10 == 0:
            continue

        img_bin = preprocess_img(img, False)
        min_area = img_bin.shape[0] * img.shape[1] / (25 * 25)
        rects = contour_detect(img_bin, min_area=min_area)
        img_bbx = img.copy()

        for rect in rects:
            xc = int(rect[0] + rect[2] / 2)
            yc = int(rect[1] + rect[3] / 2)
            size = max(rect[2], rect[3])
            x1 = max(0, int(xc - size / 2))
            y1 = max(0, int(yc - size / 2))
            x2 = min(cols, int(xc + size / 2))
            y2 = min(rows, int(yc + size / 2))

            if rect[2] > 100 and rect[3] > 100:
                cv2.rectangle(
                    img_bbx,
                    (rect[0], rect[1]),
                    (rect[0] + rect[2], rect[1] + rect[3]),
                    (0, 0, 255),
                    2,
                )

            crop_img = np.asarray(img[y1:y2, x1:x2])
            crop_img = cv2.resize(crop_img, (32, 32))
            crop_img = preprocessing(crop_img)
            crop_img = crop_img.reshape(1, 32, 32, 1)

            predictions = model.predict(crop_img)
            classIndex = np.argmax(predictions, axis=-1)[0]
            probabilityValue = np.amax(predictions)

            if probabilityValue > threshold:
                label, new_speed = get_class_name(classIndex)
                if label is not None:
                    max_speed = new_speed

        cv2.imshow("Output", img_bbx)
        print("Current Speed Limit:", max_speed)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

    esp.close()
    cap.release()
    cv2.destroyAllWindows()
