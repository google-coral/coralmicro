#!/usr/bin/python3

import argparse
import collections
import contextlib
import enum
import ipaddress
import json
import requests
import socket
import threading
import time
import tkinter as tk

from PIL import ImageTk, Image

POSENET_INPUT_WIDTH = 481
POSENET_INPUT_HEIGHT = 353

Keypoint = collections.namedtuple('Keypoint', ('point', 'score'))
Point = collections.namedtuple('Point', ('x', 'y'))
Pose = collections.namedtuple('Pose', ('keypoints', 'score'))

class KeypointType(enum.IntEnum):
    NOSE = 0
    LEFT_EYE = 1
    RIGHT_EYE = 2
    LEFT_EAR = 3
    RIGHT_EAR = 4
    LEFT_SHOULDER = 5
    RIGHT_SHOULDER = 6
    LEFT_ELBOW = 7
    RIGHT_ELBOW = 8
    LEFT_WRIST = 9
    RIGHT_WRIST = 10
    LEFT_HIP = 11
    RIGHT_HIP = 12
    LEFT_KNEE = 13
    RIGHT_KNEE = 14
    LEFT_ANKLE = 15
    RIGHT_ANKLE = 16

EDGES = (
    (KeypointType.NOSE, KeypointType.LEFT_EYE),
    (KeypointType.NOSE, KeypointType.RIGHT_EYE),
    (KeypointType.NOSE, KeypointType.LEFT_EAR),
    (KeypointType.NOSE, KeypointType.RIGHT_EAR),
    (KeypointType.LEFT_EAR, KeypointType.LEFT_EYE),
    (KeypointType.RIGHT_EAR, KeypointType.RIGHT_EYE),
    (KeypointType.LEFT_EYE, KeypointType.RIGHT_EYE),
    (KeypointType.LEFT_SHOULDER, KeypointType.RIGHT_SHOULDER),
    (KeypointType.LEFT_SHOULDER, KeypointType.LEFT_ELBOW),
    (KeypointType.LEFT_SHOULDER, KeypointType.LEFT_HIP),
    (KeypointType.RIGHT_SHOULDER, KeypointType.RIGHT_ELBOW),
    (KeypointType.RIGHT_SHOULDER, KeypointType.RIGHT_HIP),
    (KeypointType.LEFT_ELBOW, KeypointType.LEFT_WRIST),
    (KeypointType.RIGHT_ELBOW, KeypointType.RIGHT_WRIST),
    (KeypointType.LEFT_HIP, KeypointType.RIGHT_HIP),
    (KeypointType.LEFT_HIP, KeypointType.LEFT_KNEE),
    (KeypointType.RIGHT_HIP, KeypointType.RIGHT_KNEE),
    (KeypointType.LEFT_KNEE, KeypointType.LEFT_ANKLE),
    (KeypointType.RIGHT_KNEE, KeypointType.RIGHT_ANKLE),
)

def parse_keypoints(keypoints):
    return {KeypointType(i) : Keypoint(Point(x, y), score)
            for i, (score, x, y) in enumerate(keypoints)}

def parse_poses(poses):
    return [Pose(parse_keypoints(pose['keypoints']), pose['score']) for pose in poses]

def fetch_data(done, device_ip, update_image, update_poses, fps_target=10):
    thread_start = time.monotonic()
    last_report = time.monotonic()
    
    frame_count = 0
    while not done.is_set():
        start = time.monotonic()
        try:
            r = requests.get('http://%s/camera' % device_ip, timeout=1)
            if r.status_code == requests.codes.ok and r.content:
                rgb_image = Image.frombytes('RGB', (POSENET_INPUT_WIDTH, POSENET_INPUT_HEIGHT), r.content)
                update_image(rgb_image)
                frame_count += 1

            r = requests.get('http://%s/pose' % device_ip, timeout=1)
            if r.status_code == requests.codes.ok:
                update_poses(parse_poses(json.loads(r.content)))
        except socket.error:
            pass  # These happen occasionally... nothing to do but try next cycle.

        end = time.monotonic()
        if end - last_report > 5:
            fps = frame_count / (end - thread_start)
            last_report = end
            print('FPS: %f Total frames: %d Time: %f' % (fps, frame_count, (end - thread_start)))

        time.sleep(max(0, (1.0 / fps_target) - (end - start)))

@contextlib.contextmanager
def fetch(*args):
    done = threading.Event()
    thread = threading.Thread(target=lambda: fetch_data(done, *args))
    thread.start()
    try:
        yield
    finally:
        done.set()
        thread.join()
 
def main():
    parser = argparse.ArgumentParser(description='Valiant OOBE visualizer',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--device_ip_address', type=ipaddress.ip_address, default='10.10.10.1')
    args = parser.parse_args()

    root = tk.Tk()
    root.title("Valiant OOBE")
    root.columnconfigure(0, weight=1)
    root.rowconfigure(0, weight=1)
    canvas = tk.Canvas(root, width=POSENET_INPUT_WIDTH, height=POSENET_INPUT_HEIGHT)
    image_id = canvas.create_image(0, 0, anchor='nw')
    canvas.grid(column=0, row=0, sticky='nwes')

    tk_image = None
    def update_image(image):
        nonlocal tk_image
        tk_image = ImageTk.PhotoImage(image=image)
        canvas.itemconfigure(image_id, image=tk_image)        

    def update_poses(poses, r=5, threshold=0.2):
        canvas.delete('pose')
        for pose in poses:
            for start, end in EDGES:
                s = pose.keypoints[start]
                e = pose.keypoints[end]
                if s.score < threshold or e.score < threshold:
                    continue
                canvas.create_line(s.point.x, s.point.y, e.point.x, e.point.y, 
                                   fill='yellow', tags=('pose',))

            for keypoint in pose.keypoints.values():
                if keypoint.score < threshold: 
                    continue
                x, y = keypoint.point
                canvas.create_oval(x - r, y - r, x + r, y + r, 
                                   fill='cyan', outline='yellow', tags=('pose',))


    with fetch(args.device_ip_address, 
               lambda image: root.after(0, lambda: update_image(image)), 
               lambda poses: root.after(0, lambda: update_poses(poses))):
        try:
            root.mainloop()
        except KeyboardInterrupt:
            pass

if __name__ == '__main__':
    main()
