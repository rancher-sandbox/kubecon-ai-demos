from tokenize import String
import cv2
import mediapipe as mp
import numpy as np
import asyncio
from nats.aio.client import Client as NATS
from nats.errors import ConnectionClosedError, TimeoutError, NoServersError
import logging
import argparse

max_num_hands = 1
gesture = {
    0: "fist",
    1: "one",
    2: "two",
    3: "three",
    4: "four",
    5: "five",
    6: "six",
    7: "rock",
    8: "spiderman",
    9: "yeah",
    10: "ok",
}
rps_gesture = {0: "rock", 5: "paper", 9: "scissors"}


async def run(rtsp_input_url, nats_server_url, loop):
    nc = NATS()

    async def disconnected_cb():
        print("Got disconnected...")

    async def reconnected_cb():
        print("Got reconnected...")

    await nc.connect(
        nats_server_url,
        reconnected_cb=reconnected_cb,
        disconnected_cb=disconnected_cb,
        max_reconnect_attempts=-1,
    )

    # MediaPipe hands model
    mp_hands = mp.solutions.hands
    # mp_drawing = mp.solutions.drawing_utils
    hands = mp_hands.Hands(
        max_num_hands=max_num_hands,
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5,
    )

    # Gesture recognition model
    file = np.genfromtxt("training_data.csv", delimiter=",")
    angle = file[:, :-1].astype(np.float32)
    label = file[:, -1].astype(np.float32)
    knn = cv2.ml.KNearest_create()
    knn.train(angle, cv2.ml.ROW_SAMPLE, label)

    # cap = cv2.VideoCapture(0)
    cap = cv2.VideoCapture(rtsp_input_url)

    while cap.isOpened():
        ret, img = cap.read()
        if not ret:
            print("Capture read returned None")
            break

        img = cv2.flip(img, 1)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        result = hands.process(img)

        img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)

        if result.multi_hand_landmarks is not None:
            for res in result.multi_hand_landmarks:
                joint = np.zeros((21, 3))
                for j, lm in enumerate(res.landmark):
                    joint[j] = [lm.x, lm.y, lm.z]

                # Compute angles between joints
                v1 = joint[
                    [
                        0,
                        1,
                        2,
                        3,
                        0,
                        5,
                        6,
                        7,
                        0,
                        9,
                        10,
                        11,
                        0,
                        13,
                        14,
                        15,
                        0,
                        17,
                        18,
                        19,
                    ],
                    :,
                ]  # Parent joint
                v2 = joint[
                    [
                        1,
                        2,
                        3,
                        4,
                        5,
                        6,
                        7,
                        8,
                        9,
                        10,
                        11,
                        12,
                        13,
                        14,
                        15,
                        16,
                        17,
                        18,
                        19,
                        20,
                    ],
                    :,
                ]  # Child joint
                v = v2 - v1  # [20,3]
                # Normalize v
                v = v / np.linalg.norm(v, axis=1)[:, np.newaxis]

                # Get angle using arcos of dot product
                angle = np.arccos(
                    np.einsum(
                        "nt,nt->n",
                        v[[0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 16, 17, 18], :],
                        v[[1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 17, 18, 19], :],
                    )
                )  # [15,]

                angle = np.degrees(angle)  # Convert radian to degree

                # Inference gesture
                data = np.array([angle], dtype=np.float32)
                ret, results, neighbours, dist = knn.findNearest(data, 3)
                idx = int(results[0][0])

                # Draw gesture result
                if idx in rps_gesture.keys():
                    human_move = rps_gesture[idx].upper()
                    try:
                        response = await nc.publish(
                            "human_move", human_move.encode("utf-8")
                        )
                    except Exception as e:
                        print("Error:", e)
                    await nc.flush()
                    logging.info("Detected {}".format(human_move))
                # mp_drawing.draw_landmarks(img, res, mp_hands.HAND_CONNECTIONS)

        # cv2.imshow('Game', img)
        # if cv2.waitKey(1) == ord('q'):
        #     break

    # Terminate connection to NATS.
    await nc.drain()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("rtsp_input_url")
    parser.add_argument("nats_server_url")
    args = vars(parser.parse_args())

    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(args["rtsp_input_url"], args["nats_server_url"], loop))
    # loop.run_forever()
    loop.close()

# python3 determine_human_move.py rtsp://localhost:8554/rps nats://localhost:4222
