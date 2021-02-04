import Foundation
import AppPotSDKforSwift

@objc(BlackboardCamera) class BlackboardCamera : CDVPlugin {

  // 写真撮影
  @objc(capture:) func capture(_ command: CDVInvokedUrlCommand) {
        let storyboard = UIStoryboard(name: "Main", bundle: nil)
        if let vc: CameraViewController  = storyboard.instantiateInitialViewController() as? CameraViewController {
            vc.modalPresentationStyle = UIModalPresentationStyle.fullScreen
            vc.callbackId = command.callbackId
            vc.commandDelegate = self.commandDelegate
            if let base64 = command.argument(at: 0) as? String {
                vc.boardImage = String2Image(base64String: base64)
            }
            if let isNeedBlackBoard = command.argument(at: 1) as? Bool {
                vc.isNeedBlackBoard = isNeedBlackBoard
            }
            if let blackboardViewPriority = command.argument(at: 2) as? String {
                vc.blackboardViewPriority = blackboardViewPriority
            }
            guard let authToken = command.argument(at: 3) as? String else {
                invoke(callbackId: command.callbackId, commandDelegate: self.commandDelegate, data: "Close", mode: vc.blackboardViewPriority!)
                return
            }
            initAppPotSDK(authToken: authToken)

            // PictureData情報をセットする
            if let data = command.argument(at: 4) as? String {
                let pictureData = makePictureData(jsonString: data)
                if let data = pictureData {
                    vc.pictureData = data
                }
            }
            self.viewController.present(vc, animated: true, completion: nil)
        }
    }

    func invoke(callbackId:String, commandDelegate:CDVCommandDelegate, data:String, mode:String) {
        let pluginResultInInvoke = CDVPluginResult(status: CDVCommandStatus_OK, messageAs: ["mode": mode, "data": data])
        commandDelegate.send(pluginResultInInvoke, callbackId: callbackId)
    }
    
    func makePictureData(jsonString: String) -> Dictionary<String, Any>? {
        let jsonObj = jsonString.data(using: .utf8)!
        do {
            let pictureJson = try JSONSerialization.jsonObject(with: jsonObj) as! Dictionary<String, Any>
            return pictureJson
        } catch {
            print(error)
        }
        return nil
    }
    
    //StringをUIImageに変換する
    func String2Image(base64String: String) -> UIImage? {

        let results = base64String.matches(for: "data:image\\/([a-zA-Z]*);base64,([^\\\"]*)")
        for imageString in results {
            if let image = imageString.base64ToImage() {
                return image
            }
        }
        return nil
    }
    fileprivate func initAppPotSDK(authToken: String) {
        let modelClasses: [AnyClass] = [type(of: PictureData())]
        let configuration = AppPot.Configuration(companyId: 63,
                                                 appVersion: "1.0",
                                                 appKey: "b40b0497304642008a9fe808a7bbd9a3",
                                                 appID: "taisei_kui_develop",
                                                 hostName: "trial.apppot.net",
                                                 contextRoot: "/apppot",
                                                 portNumber: 80,
                                                 customHeaderFields: nil,
                                                 useSSL: false,
                                                 usePushNotification: true,
                                                 maximumNumberOfLog: 300,
                                                 retryInterbalOfFailedMessages: 600,
                                                 hasLimitedSessionTime: false,
                                                 models: modelClasses)
        
        print("===== Before Call ========")
        AppPot.prepareToUse(with: configuration,
                            recreateDatabase: true,
                            onCompletion: { print("---- Succeed to prepare AppPot.")},
                            onError: {error in print("---- Faild to prepare AppPot : \(error)")})
        if let appInfo = APAuthInfo.init(json: authToken) {
            print("appInfo.authToken=\(String(describing: appInfo.authToken))")
            APService.sharedInstance()?.authInfo = appInfo
        } else {
            print("[Error] no appInfo!!!!!!!!\(authToken)")
        }
        
    }
}

public extension String {
    func base64ToImage() -> UIImage? {
        if let url = URL(string: self),let data = try? Data(contentsOf: url),let image = UIImage(data: data) {
            return image
        }
        return nil
    }
    
    func matches(for regex: String) -> [String] {
        do {
            let regex = try NSRegularExpression(pattern: regex)
            let results = regex.matches(in: self, range:  NSRange(self.startIndex..., in: self))
            return results.map {
                //self.substring(with: Range($0.range, in: self)!)
                String(self[Range($0.range, in: self)!])
            }
        } catch let error {
            print("invalid regex: \(error.localizedDescription)")
            return []
        }
    }
}
