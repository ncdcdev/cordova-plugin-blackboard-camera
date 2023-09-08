/*global cordova, module*/

module.exports = {
  getPicture: function (
    base64,
    isNeedBlackBoard,
    blackboardViewPriority,
    jcomsiaPhoto,
    version,
    successCallback,
    errorCallback
  ) {
    cordova.exec(
      successCallback,
      errorCallback,
      "BlackboardCamera",
      "capture",
      [base64, isNeedBlackBoard, blackboardViewPriority, jcomsiaPhoto, version]
    );
  },
};
