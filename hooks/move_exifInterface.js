var fs = require("fs");
var path = require("path");
var Q = require("q");

module.exports = function (context) {
  var deferral = new Q.defer();
  var targetFiles = ["ExifInterfaceFix.java", "ExifInterfaceUtils.java"];

  targetFiles.map(fileName => {
    var sourceFile = path.join(
      context.opts.projectRoot,
      `platforms/android/app/src/main/java/jp/co/taisei/construction/fieldmanagement/plugin/${fileName}`
    );
    var targetFile = path.join(
      context.opts.projectRoot,
      `platforms/android/app/src/main/java/androidx/exifinterface/media/${fileName}`
    );

    fs.rename(sourceFile, targetFile, function (err) {
      if (err) {
        deferral.reject("Error moving file: " + err);
      } else {
        console.log("File moved successfully");
        deferral.resolve();
      }
    });
  });

  return deferral.promise;
};
