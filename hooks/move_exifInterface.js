var fs = require("fs");
var path = require("path");

module.exports = function (context) {
  var Q = context.requireCordovaModule("q");
  var deferral = new Q.defer();

  var sourceFile = path.join(
    context.opts.projectRoot,
    "platforms/android/app/src/main/java/jp/co/taisei/construction/fieldmanagement/plugin/ExifInterface2.java"
  );
  var targetFile = path.join(
    context.opts.projectRoot,
    "platforms/android/app/src/main/java/androidx/exifinterface/media/ExifInterface2.java"
  );

  fs.rename(sourceFile, targetFile, function (err) {
    if (err) {
      deferral.reject("Error moving file: " + err);
    } else {
      console.log("File moved successfully");
      deferral.resolve();
    }
  });

  return deferral.promise;
};
