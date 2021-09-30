#!/usr/bin/python3

import argparse
import cairosvg
import collections
import enum
import ipaddress
import requests
import socket
import svgwrite
import threading
import tkinter as tk
from io import BytesIO
from copy import deepcopy
from flask import Flask, current_app
from flask import request as flask_request
from jsonrpc.backend.flask import api
from PIL import ImageTk, Image

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
        self.host_ip = self.get_own_address()
        self.panel = None
        self.frames = {}
        self.poses = {}
        self.poses_count = {}

    def get_next_id(self):
        next_id = self.next_id
        self.next_id += 1
        return next_id

    def get_new_payload(self):
        new_payload = deepcopy(self.payload_template)
        new_payload['id'] = self.get_next_id()
        return new_payload

    def send_rpc(self, json):
        if json['method'] and (json['jsonrpc'] == '2.0') and (json['id'] != -1) and (json['params'] != None):
            if self.print_payloads:
                print(json)
            return requests.post(self.url, json=json).json()
        raise ValueError('Missing key in RPC')

    def get_own_address(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect((str(self.device_ip), self.device_port))
        own_address = s.getsockname()[0]
        s.close()
        return own_address

    def register_receiver(self):
        payload = self.get_new_payload()
        payload['method'] = 'register_receiver'
        payload['params'].append({
            'address': self.get_own_address(),
        })
        return self.send_rpc(payload)

    def unregister_receiver(self):
        payload = self.get_new_payload()
        payload['method'] = 'unregister_receiver'
        payload['params'].append({
            'address': self.get_own_address(),
        })
        return self.send_rpc(payload)

    def render_frame(self, index):
        final_frame = self.frames[index].copy()
        local_poses = self.poses[index]
        for pose in local_poses:
            final_frame.alpha_composite(pose)
        self.poses.pop(index)
        self.poses_count.pop(index)
        self.frames.pop(index)

        tk_image = ImageTk.PhotoImage(image=final_frame)
        self.panel.configure(image=tk_image)
        self.panel.image = tk_image

    def rpc_pose(self, *args, **kwargs):
        pose_keypoints = {}
        for i, value in enumerate(kwargs['keypoints']):
            name = KeypointType(i)
            score = value[0]
            x = value[1]
            y = value[2]
            pose_keypoints[name] = Keypoint(Point(x, y), score)
        pose = Pose(pose_keypoints, kwargs['score'])

        dwg = svgwrite.Drawing('', size=(481, 353))
        draw_pose(dwg, pose, (481, 353), (0, 0, 481, 353))

        png = cairosvg.svg2png(bytestring=dwg.tostring().encode())
        pose_image = Image.open(BytesIO(png))
        self.poses[kwargs['now']].append(pose_image)
        if len(self.poses[kwargs['now']]) == self.poses_count[kwargs['now']]:
            self.render_frame(kwargs['now'])

        return []

    def rpc_frame(self, *args, **kwargs):
        r = requests.get('http://' + str(self.device_ip) + kwargs['url'])
        rgb_image = Image.frombytes('RGB', (481, 353), r.content)
        rgb_image.putalpha(255)
        self.frames[kwargs['now']] = rgb_image
        self.poses[kwargs['now']] = []
        self.poses_count[kwargs['now']] = kwargs['poses']
        if (kwargs['poses'] == 0):
            self.render_frame(kwargs['now'])

        return []


@api.dispatcher.add_method
def pose(*args, **kwargs):
    oobe = current_app.oobe
    return oobe.rpc_pose(*args, **kwargs)

@api.dispatcher.add_method
def frame(*args, **kwargs):
    oobe = current_app.oobe
    return oobe.rpc_frame(*args, **kwargs)

def main():
    parser = argparse.ArgumentParser(description='Valiant OOBE visualizer',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--device_ip_address', type=str, default='10.10.10.1')
    args = parser.parse_args()

    app = Flask(__name__)
    app.register_blueprint(api.as_blueprint())

    @app.route("/shutdown", methods=['POST'])
    def shutdown():
        func = flask_request.environ.get('werkzeug.server.shutdown')
        func()
        return "Shutting down"

    oobe = ValiantOOBE(ipaddress.ip_address(args.device_ip_address), 80, print_payloads=True)
    oobe.register_receiver()
    app.oobe = oobe

    flask_thread = threading.Thread(target=lambda: app.run(host=oobe.host_ip, port='8080', debug=True, use_reloader=False))
    flask_thread.start()

    window = tk.Tk()
    window.title("Valiant OOBE")
    window.geometry("481x353")
    window.configure(background='grey')

    oobe.panel = tk.Label(window)
    oobe.panel.pack()
    try:
        window.mainloop()
    except KeyboardInterrupt:
        pass

    oobe.unregister_receiver()
    requests.post('http://' + oobe.host_ip + ':8080/shutdown')
    flask_thread.join()

if __name__ == '__main__':
    main()
