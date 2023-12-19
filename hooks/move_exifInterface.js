var fs = require("fs");
var path = require("path");
var Q = require("q");

module.exports = function (context) {
  console.log("[move_exifInterface]start");
  var deferral = new Q.defer();
  var targetFiles = ["ExifInterfaceFix.java", "ExifInterfaceUtils.java"];

  targetFiles.map(fileName => {
    console.log(`[move_exifInterface]map:${fileName}`);
    var sourceFile = path.join(
      context.opts.projectRoot,
      `platforms/android/app/src/main/java/jp/co/taisei/construction/fieldmanagement/plugin/${fileName}`
    );
    var targetDir = path.join(
      context.opts.projectRoot,
      "platforms/android/app/src/main/java/androidx/exifinterface/media"
    );
    var targetFile = path.join(targetDir, fileName);

    // コピー先ディレクトリが存在するか確認し、存在しない場合は作成する
    if (!fs.existsSync(targetDir)) {
      fs.mkdir(targetDir, { recursive: true });
    }

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
