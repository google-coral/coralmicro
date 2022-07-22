/*
 Copyright 2022 Google LLC
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

const titleContainerHeight = 45;
const logConsoleHeight = 150;
const settingMenuHeight = 100;
const windowWidth = 1005;
let currentImageWidth = 0;
let currentImageHeight = 0;
let windowHeight = 900;

/**
 * Calls the video_feed function from python.
 */
function startVideoFeed() {
    eel.video_feed()()
}

/**
 * Toggles the setting menu element.
 * @param  {[Element]} settingMenuElement The setting menu element.
 */
function toggleSettingMenu(settingMenuElement) {
    let show = settingMenuElement.classList.toggle('change');
    if (show) {
        document.getElementById('setting-menu').style.display = '';
    } else {
        document.getElementById('setting-menu').style.display = 'none';
    }
}


/**
 * Updates the image.
 *
 * This function gets called by python to update the new image as well as resizing the window to fit the image.
 * @param  {[String]} imgSrc The image's source file.
 * @param  {[Int]} imgWidth The image's width.
 * @param  {[Int]} imgHeight The image's height.
 */
eel.expose(updateImageSrc);
function updateImageSrc(imgSrc, imgWidth, imgHeight) {
    currentImageHeight = imgHeight;
    currentImageWidth = imgWidth;

    let videoElement = document.getElementById('video-feed');
    videoElement.src = imgSrc
    videoElement.width = imgWidth
    videoElement.height = imgHeight

    windowHeight = currentImageHeight + titleContainerHeight + logConsoleHeight + 35;
    if (document.getElementById('setting-menu').style.display === '') {
       windowHeight += settingMenuHeight;
    }
    window.resizeTo(windowWidth, windowHeight)
}

/**
 * Updates the logging console with the new message.
 *
 * @param  {[String]} msg The message to update to the logging console.
 */
eel.expose(updateLog);
function updateLog(msg) {
    let elt = document.getElementById('log-console');
    elt.value += '\n' + msg;
    elt.scrollTop = elt.scrollHeight;
}

/**
 * Collects all of the image settings inputs from user and return it in json format.
 */
eel.expose(getImageConfig);
function getImageConfig() {
    let config = {};
    config.rotation = document.getElementById('rotation-selector').value;
    config.format = document.getElementById('format-selector').value;
    if (config.format === "RAW") {
        config.height = 324
        config.width = 324
        config.rotation = 0
        document.getElementById('image-width').value = 324
        document.getElementById('image-height').value = 324
        document.getElementById('rotation-selector').value = 0;
    }
    config.width = document.getElementById('image-width').value;
    if (config.width === "" || config.width < 1) {
        updateLog('ERROR: Width must be at least 1');
        config.width = 1
    } else if (config.width > 1000) {
        config.width = 1000
        updateLog('ERROR: Width must be less than 1000');
    }
    config.height = document.getElementById('image-height').value;
    if (config.height === "" || config.height < 1) {
        updateLog('ERROR: Height must be at least 1');
        config.height = 1
    } else if (config.height > 1000) {
        updateLog('ERROR: Height must be less than 1000');
        config.height = 1000
    }
    config.filter = document.getElementById('filter-selector').value;
    config.awb = !!document.getElementById('auto-white-balance').checked;
    return config;
}