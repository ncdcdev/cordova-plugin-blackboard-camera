var fs = require("fs-extra");
var path = require("path");

module.exports = function (context) {
  var srcDir = path.join(
    context.opts.projectRoot,
    "plugins",
    "cordova-plugin-blackboard-camera",
    "src",
    "android",
    "cpp"
  );
  var destDir = path.join(
    context.opts.projectRoot,
    "platforms",
    "android",
    "app",
    "src",
    "main",
    "cpp"
  );

  fs.copySync(srcDir, destDir);
};
