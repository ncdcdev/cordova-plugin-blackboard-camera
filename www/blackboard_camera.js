/*global cordova, module*/

module.exports = {
    getPicture: function (base64, isNeedBlackBoard, blackboardViewPriority, authToken, pictureData, successCallback, errorCallback) {
        setTimeout(() => {
            console.log(`getPicture:authToken:${authToken}`);
            cordova.exec(successCallback, errorCallback, "BlackboardCamera", "capture", [base64, isNeedBlackBoard, blackboardViewPriority, authToken, pictureData]);
        }, 10);
    }
};