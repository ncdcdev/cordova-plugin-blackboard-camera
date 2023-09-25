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

    // C++とKotlinの連携の設定を追加
    var settingsToAdd = `
android {
    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
        }
    }
}
`;

    if (!gradleContent.includes("externalNativeBuild")) {
      gradleContent = gradleContent.replace(
        "android {",
        "android {" + settingsToAdd
      );
      fs.writeFileSync(gradlePath, gradleContent, "utf-8");
    }
  }
};
