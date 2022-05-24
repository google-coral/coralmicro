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

POSENET_INPUT_WIDTH = 324
POSENET_INPUT_HEIGHT = 324

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
    last_pose_report = time.monotonic()

    frame_count = 0
    blank_frames = 0
    while not done.is_set():
        start = time.monotonic()
        try:
            r = requests.get('http://%s/camera' % device_ip, timeout=1)
            if r.status_code == requests.codes.ok and r.content:
                rgb_image = Image.frombytes('RGB', (POSENET_INPUT_WIDTH, POSENET_INPUT_HEIGHT), r.content)
                update_image(rgb_image)
                frame_count += 1
                blank_frames = 0
            elif r.status_code == requests.codes.not_found:
                blank_frames += 1
                if (blank_frames > 10): # In case low power mode immediately exits
                    update_image(None)
                    blank_frames = 0

            r = requests.get('http://%s/pose' % device_ip, timeout=1)
            if r.status_code == requests.codes.ok:
                last_pose_report = time.monotonic()
                update_poses(parse_poses(json.loads(r.content)))

            if time.monotonic() - last_pose_report > 2:
                update_poses([])
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
    parser = argparse.ArgumentParser(description='Dev Board Micro OOBE visualizer',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--device_ip_address', type=ipaddress.ip_address, default='10.10.10.1')
    parser.add_argument('--mirror', action='store_true')
    parser.add_argument('--fullscreen', action='store_true')
    args = parser.parse_args()

    root = tk.Tk()
    root.title("Dev Board Micro OOBE")
    root.columnconfigure(0, weight=1)
    root.rowconfigure(0, weight=1)
    if args.fullscreen:
        root.attributes('-fullscreen', True)
        w, h = root.winfo_screenwidth(), root.winfo_screenheight()
        (w,h) = (h,h) if h < w else (w,w)
        scale_x = w / POSENET_INPUT_WIDTH
        scale_y = h / POSENET_INPUT_HEIGHT
        start_x = (root.winfo_screenwidth() - w) / 2
        end_x = start_x + w
        canvas = tk.Canvas(root, highlightthickness=0, bg='black')
    else:
        canvas = tk.Canvas(root, width=POSENET_INPUT_WIDTH, height=POSENET_INPUT_HEIGHT, bg='black')
        scale_x = 1
        scale_y = 1
        start_x = 0
        start_y = 0
        w = POSENET_INPUT_WIDTH
        h = POSENET_INPUT_HEIGHT
        end_x = POSENET_INPUT_WIDTH

    image_id = canvas.create_image(start_x, 0, anchor='nw')
    text_id = canvas.create_text(start_x + w/2, h/2, font=f"Helvetica {int(h/20)} bold", fill='white', text="Waiting for Person Detection", state=tk.HIDDEN)
    canvas.grid(column=0, row=0, sticky='nwes')

    tk_image = None
    def update_image(image):
        nonlocal tk_image
        if not image:
            canvas.itemconfigure(text_id, state=tk.NORMAL)
            canvas.itemconfigure(image_id, state=tk.HIDDEN)
            canvas.delete('pose')
            return

        canvas.itemconfigure(text_id, state=tk.HIDDEN)
        canvas.itemconfigure(image_id, state=tk.NORMAL)

        if args.fullscreen:
            if args.mirror:
                tk_image = ImageTk.PhotoImage(image=(image.resize((w,h)).transpose(Image.FLIP_LEFT_RIGHT)))
            else:
                tk_image = ImageTk.PhotoImage(image=(image.resize((w,h))))
        else:
            if args.mirror:
                tk_image = ImageTk.PhotoImage(image=image.transpose(Image.FLIP_LEFT_RIGHT))
            else:
                tk_image = ImageTk.PhotoImage(image=image)

        canvas.itemconfigure(image_id, image=tk_image)

    def get_pixel_location(x,y, r=0):
        if args.fullscreen:
            if args.mirror:
                return end_x - (x * scale_x) + r, (y * scale_y) + r
            else:
                return start_x + (x * scale_x) + r, (y * scale_y) + r
        else:
            if args.mirror:
                return POSENET_INPUT_WIDTH - x + r, y + r
            else:
                return x + r, y + r

    def update_poses(poses, r=5, threshold=0.2):
        canvas.delete('pose')
        for pose in poses:
            for start, end in EDGES:
                s = pose.keypoints[start]
                e = pose.keypoints[end]
                if s.score < threshold or e.score < threshold:
                    continue
                canvas.create_line(get_pixel_location(s.point.x, s.point.y),
                    get_pixel_location(e.point.x, e.point.y), fill='yellow', tags=('pose',))

            for keypoint in pose.keypoints.values():
                if keypoint.score < threshold: 
                    continue
                x, y = keypoint.point
                canvas.create_oval(get_pixel_location(x,y,-r), get_pixel_location(x,y,r),
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
