#!/usr/bin/python3

import argparse
import cairosvg
import collections
import enum
import ipaddress
import json
import requests
import socket
import svgwrite
import threading
import time
import tkinter as tk
from io import BytesIO
from copy import deepcopy
from PIL import ImageTk, Image

POSENET_INPUT_WIDTH = 481
POSENET_INPUT_HEIGHT = 353

Keypoint = collections.namedtuple('Keypoint', ['point', 'score'])
Point = collections.namedtuple('Point', ['x', 'y'])
Pose = collections.namedtuple('Pose', ['keypoints', 'score'])

class KeypointType(enum.IntEnum):
    """Pose kepoints."""
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

def draw_pose(dwg, pose, src_size, inference_box, color='yellow', threshold=0.2):
    box_x, box_y, box_w, box_h = inference_box
    scale_x, scale_y = src_size[0] / box_w, src_size[1] / box_h
    xys = {}
    for label, keypoint in pose.keypoints.items():
        if keypoint.score < threshold: continue
        # Offset and scale to source coordinate space.
        kp_x = int((keypoint.point[0] - box_x) * scale_x)
        kp_y = int((keypoint.point[1] - box_y) * scale_y)

        xys[label] = (kp_x, kp_y)
        dwg.add(dwg.circle(center=(int(kp_x), int(kp_y)), r=5,
                           fill='cyan', fill_opacity=keypoint.score, stroke=color))

    for a, b in EDGES:
        if a not in xys or b not in xys: continue
        ax, ay = xys[a]
        bx, by = xys[b]
        dwg.add(dwg.line(start=(ax, ay), end=(bx, by), stroke=color, stroke_width=2))

class ValiantOOBE(object):
    def __init__(self, device_ip, device_port, print_payloads=False):
        self.device_ip = device_ip
        self.device_port = device_port
        self.url = 'http://' + str(device_ip) + ':' + str(device_port) + '/jsonrpc'
        self.print_payloads = print_payloads
        self.next_id = 0
        self.payload_template = {
            'jsonrpc': '2.0',
            'params': [],
            'method': '',
            'id': -1,
        }
        self.panel = None
        self.frame = None
        self.poses = []

    def render_frame(self):
        final_frame = self.frame.copy()
        for pose in self.poses:
            final_frame.alpha_composite(pose)

        tk_image = ImageTk.PhotoImage(image=final_frame)
        self.panel.configure(image=tk_image)
        self.panel.image = tk_image

    def pose(self, pose_json):
        try:
            poses = json.loads(pose_json)
            for pose in poses:
                pose_keypoints = {}
                for i, value in enumerate(pose['keypoints']):
                    name = KeypointType(i)
                    score = value[0]
                    x = value[1]
                    y = value[2]
                    pose_keypoints[name] = Keypoint(Point(x, y), score)
                p = Pose(pose_keypoints, pose['score'])

                dwg = svgwrite.Drawing('', size=(POSENET_INPUT_WIDTH, POSENET_INPUT_HEIGHT))
                draw_pose(dwg, p, (POSENET_INPUT_WIDTH, POSENET_INPUT_HEIGHT), (0, 0, POSENET_INPUT_WIDTH, POSENET_INPUT_HEIGHT))
                png = cairosvg.svg2png(bytestring=dwg.tostring().encode())
                pose_image = Image.open(BytesIO(png))
                self.poses.append(pose_image)

        except Exception as e:
            print('exception ' + str(e))

def FetchThread(shutdown_event, oobe):
    thread_start = time.time()
    last_report = time.time()
    frame_count = 0
    while not shutdown_event.is_set():
        start = time.time()
        try:
            r = requests.get('http://' + str(oobe.device_ip) + '/camera', timeout=1)
            if len(r.content) > 0:
                rgb_image = Image.frombytes('RGB', (POSENET_INPUT_WIDTH, POSENET_INPUT_HEIGHT), r.content)
                rgb_image.putalpha(255)
                oobe.frame = rgb_image
                frame_count += 1
            r2 = requests.get('http://' + str(oobe.device_ip) + '/pose', timeout=1)
            if r2.status_code == requests.codes.ok:
                oobe.poses = []
                oobe.pose(r2.content)
        # These happen occasionally... nothing to do but try next cycle.
        except socket.error:
            pass

        if oobe.frame:
            oobe.render_frame()

        end = time.time()
        if (end - last_report) > 10:
            frames_per_second = frame_count / (end - thread_start)
            last_report = end
            print('FPS: %f Total frames: %d Time: %f' % (frames_per_second, frame_count, (end - thread_start)))

        fps_target = 10
        sleep_time = (1.0 / fps_target) - (end - start)
        if (sleep_time > 0):
            time.sleep(sleep_time)

def main():
    parser = argparse.ArgumentParser(description='Valiant OOBE visualizer',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--device_ip_address', type=str, default='10.10.10.1')
    args = parser.parse_args()

    oobe = ValiantOOBE(ipaddress.ip_address(args.device_ip_address), 80, print_payloads=True)

    fetch_shutdown_event = threading.Event()
    fetch_thread = threading.Thread(target=lambda: FetchThread(fetch_shutdown_event, oobe))
    fetch_thread.start()

    window = tk.Tk()
    window.title("Valiant OOBE")
    window.geometry("%dx%d" % (POSENET_INPUT_WIDTH, POSENET_INPUT_HEIGHT))
    window.configure(background='grey')

    oobe.panel = tk.Label(window)
    oobe.panel.pack()
    try:
        window.mainloop()
    except KeyboardInterrupt:
        pass

    fetch_shutdown_event.set()
    fetch_thread.join()

if __name__ == '__main__':
    main()
