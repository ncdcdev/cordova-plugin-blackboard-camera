var fs = require("fs");
var path = require("path");

module.exports = function (context) {
  var gradlePath = path.join(
    context.opts.projectRoot,
    "platforms",
    "android",
    "app",
    "build.gradle"
  );

  if (fs.existsSync(gradlePath)) {
    var gradleContent = fs.readFileSync(gradlePath, "utf-8");

    if (!gradleContent.match(/externalNativeBuild/g)) {
      var placeHolder = "android {";
      var newContent = `
    externalNativeBuild {
        cmake {
            path "src/android/cpp/CMakeLists.txt"
        }
    }
`;
      gradleContent = gradleContent.replace(
        placeHolder,
        placeHolder + newContent
      );
      fs.writeFileSync(gradlePath, gradleContent, "utf-8");
    }
  }
};
