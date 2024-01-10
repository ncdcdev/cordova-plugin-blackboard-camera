var fs = require("fs").promises;
var path = require("path");

module.exports = async function (context) {
  var targetFiles = ["ExifInterfaceFix.java", "ExifInterfaceUtils.java"];

  for (const fileName of targetFiles) {
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
    try {
      await fs.access(targetDir, fs.constants.F_OK);
    } catch (err) {
      await fs.mkdir(targetDir, { recursive: true });
    }

    // ファイルの移動
    try {
      await fs.rename(sourceFile, targetFile);
      console.log("File moved successfully");
    } catch (err) {
      console.error("Error moving file: " + err);
      throw err; // エラーを外部に伝播させる
    }
  }
};
