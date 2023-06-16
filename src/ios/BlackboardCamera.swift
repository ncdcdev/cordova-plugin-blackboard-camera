import Foundation

@objc(BlackboardCamera) class BlackboardCamera : CDVPlugin {

  // 写真撮影
  @objc(capture:) func capture(_ command: CDVInvokedUrlCommand) {
        let storyboard = UIStoryboard(name: "Main", bundle: nil)
        if let vc: CameraViewController  = storyboard.instantiateInitialViewController() as? CameraViewController {
            vc.modalPresentationStyle = UIModalPresentationStyle.fullScreen
            vc.callbackId = command.callbackId
            vc.commandDelegate = self.commandDelegate
            let base64 = command.argument(at: 0) as! String
            let isNeedBlackBoard = command.argument(at: 1) as! Bool
            let blackboardViewPriority = command.argument(at: 2) as! String
            let jcomsiaPhoto = command.argument(at: 3) as! String
            vc.boardImage = String2Image(base64String: base64)
            vc.isNeedBlackBoard = isNeedBlackBoard
            vc.blackboardViewPriority = blackboardViewPriority
            do {
                let jsonData = jcomsiaPhoto.data(using: .utf8)!
                let photoInfo = try JSONSerialization.jsonObject(with: jsonData, options:  .mutableContainers)
                vc.photoInfo = PhotoInfo(dic: photoInfo as! Dictionary<String, Any>)
            } catch {
                // TODO
                print(error.localizedDescription)
            }
            self.viewController.present(vc, animated: true, completion: nil)
        }
    }

    func invoke(callbackId:String, commandDelegate:CDVCommandDelegate, data:String, mode:String) {
        let pluginResultInInvoke = CDVPluginResult(status: CDVCommandStatus_OK, messageAs: ["mode": mode, "data": data])
        commandDelegate.send(pluginResultInInvoke, callbackId: callbackId)
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
