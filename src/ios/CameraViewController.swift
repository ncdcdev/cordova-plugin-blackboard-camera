//
//  CameraViewController.swift
//  T-Pile Recorder
//
//  Created by ncdc-jang on 2019/12/03.
//

import UIKit
import AVFoundation

enum Marker: Int {
    case Inside = 0
    case LeftTop = 1
    case RightTop = 2
    case LeftBottom = 3
    case RightBottom = 4
    case None = -1
}

class CameraViewController: UIViewController {

    // Parameter
    var callbackId: String!
    var commandDelegate: CDVCommandDelegate!
    var boardImage: UIImage?
    var isNeedBlackBoard: Bool?
    var blackboardViewPriority: String?

    var ltCircle: UIView? // leftTop
    var rtCircle: UIView? // rightTop
    var lbCircle: UIView? // leftBottom
    var rbCircle: UIView? // rightBottom

    @IBOutlet weak var cameraView: UIView!
    // デバイスからの入力と出力を管理するオブジェクトの作成
    var captureSession = AVCaptureSession()
    // カメラデバイスそのものを管理するオブジェクトの作成
    // メインカメラの管理オブジェクトの作成
    var mainCamera: AVCaptureDevice?
    // インカメの管理オブジェクトの作成
    var innerCamera: AVCaptureDevice?
    // 現在使用しているカメラデバイスの管理オブジェクトの作成
    var currentDevice: AVCaptureDevice?
    // キャプチャーの出力データを受け付けるオブジェクト
    var photoOutput : AVCapturePhotoOutput?
    // プレビュー表示用のレイヤ
    var cameraPreviewLayer : AVCaptureVideoPreviewLayer?
    // シャッターボタン
    @IBOutlet weak var cameraButton: UIButton!
    // Flashイメージ
    @IBOutlet weak var flashImage: UIImageView!
    // Flash TextButon
    @IBOutlet weak var flashButton: UIButton!
    // Board Image View
    @IBOutlet weak var boardImageView: UIImageView!
    // BoardMode Text
    @IBOutlet weak var blackBoardModeText: UILabel!

    // 縦用制約
    @IBOutlet weak var vFlashButtonConstraint: NSLayoutConstraint!
    @IBOutlet weak var vFlashImageConstraint: NSLayoutConstraint!
    @IBOutlet weak var vCameraButtonConstraint: NSLayoutConstraint!
    @IBOutlet weak var vBackButtonConstraint: NSLayoutConstraint!
    @IBOutlet weak var vIconDoneConstraint: NSLayoutConstraint!
    @IBOutlet weak var vCameraButtonCenterConstraint: NSLayoutConstraint!
    @IBOutlet weak var vCameraViewAspectConstraint: NSLayoutConstraint!
    @IBOutlet weak var vCameraViewRightConstraint: NSLayoutConstraint!
    @IBOutlet weak var vCameraViewLeftConstraint: NSLayoutConstraint!
    @IBOutlet weak var vBlackBoardEdit01Constraint: NSLayoutConstraint!
    @IBOutlet weak var vBlackBoardEdit02Constraint: NSLayoutConstraint!
    @IBOutlet weak var vBlackBoardMode01Constraint: NSLayoutConstraint!
    @IBOutlet weak var vBlackBoardMode02Constraint: NSLayoutConstraint!
    
    // 横用制約
    @IBOutlet weak var hFlashButtonConstraint: NSLayoutConstraint!
    @IBOutlet weak var hFlashImageConstraint: NSLayoutConstraint!
    @IBOutlet weak var hCameraButtonConstraint: NSLayoutConstraint!
    @IBOutlet weak var hBackButtonConstraint: NSLayoutConstraint!
    @IBOutlet weak var hIconDoneConstraint: NSLayoutConstraint!
    @IBOutlet weak var hCameraButtonCenterConstraint: NSLayoutConstraint!
    @IBOutlet weak var hCameraViewAspectConstraint: NSLayoutConstraint!
    @IBOutlet weak var hCameraViewBottomConstraint: NSLayoutConstraint!
    @IBOutlet weak var hCameraViewTopConstraint: NSLayoutConstraint!
    @IBOutlet weak var hBlackBoardEdit01Constraint: NSLayoutConstraint!
    @IBOutlet weak var hBlackBoardEdit02Constraint: NSLayoutConstraint!
    @IBOutlet weak var hBlackBoardMode01Constraint: NSLayoutConstraint!
    @IBOutlet weak var hBlackBoardMode02Constraint: NSLayoutConstraint!

    var vViewConstraints: [NSLayoutConstraint]!
    var hViewConstraints: [NSLayoutConstraint]!

    private let kFlashMode = "flashMode"
    private let kBoardInitX = "boardInitX"
    private let kBoardInitY = "boardInitY"
    private let kBoardInitWidth = "boardInitWidth"
    private let kBoardMode = "boardMode"
    
    private let kBoardMinScale: CGFloat = 0.4
    private let kBoardBaseScale: CGFloat = 0.5
    private let kBoardMarkSize: CGFloat = 30

    // FlashモードIndex 0(Auto) 1(Off) 2(On)
    var flashModeIndex: Int = 0
    var boardInitPoint: CGPoint = CGPoint.zero
    var boardInitSize: CGSize = CGSize.zero
    var boardSizeMargin: CGFloat = 0
    
    var locationInit: CGPoint!
    var cameraViewMargin: CGPoint = CGPoint.zero
    var pointType: Marker?
    var currentBoardFrame: CGRect = CGRect.zero
    
    var currentVolumn: Float = Float.zero
    let audioSession = AVAudioSession.sharedInstance()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        boardSizeMargin = kBoardMarkSize / 2
        setupCaptureSession()
        setupDevice()
        setupInputOutput()
        setupPreviewLayer()
        captureSession.startRunning()
        setFromSavedData()
        
        // 縦向き用の制限の配列
        vViewConstraints = [vFlashButtonConstraint, vFlashImageConstraint, vCameraButtonConstraint, vBackButtonConstraint, vIconDoneConstraint,
        vCameraButtonCenterConstraint, vCameraViewAspectConstraint, vCameraViewRightConstraint, vCameraViewLeftConstraint,
        vBlackBoardEdit01Constraint, vBlackBoardEdit02Constraint, vBlackBoardMode01Constraint, vBlackBoardMode02Constraint]
        // 横向け用の制限の配列
        hViewConstraints = [hFlashButtonConstraint, hFlashImageConstraint, hCameraButtonConstraint, hBackButtonConstraint, hIconDoneConstraint,
        hCameraButtonCenterConstraint, hCameraViewAspectConstraint, hCameraViewBottomConstraint, hCameraViewTopConstraint,
        hBlackBoardEdit01Constraint, hBlackBoardEdit02Constraint, hBlackBoardMode01Constraint, hBlackBoardMode02Constraint]
        // 回転イベントを通知
        NotificationCenter.default.addObserver(self, selector: #selector(CameraViewController.onOrientationDidChange(notification:)), name: UIDevice.orientationDidChangeNotification, object: nil)
        // 監視を有効にする
        try! audioSession.setActive(true)
        currentVolumn = audioSession.outputVolume // 初期値を設定
    }

    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        self.cameraViewMargin = cameraMarginPoint()

        // 現在の回転で描画
        self.rotationCamera()
        // ビューのサイズの調整
        cameraPreviewLayer?.position = CGPoint(x: self.cameraView.frame.width / 2, y: self.cameraView.frame.height / 2)
        cameraPreviewLayer?.bounds = cameraView.frame
        if let image = self.boardImage {
            self.boardImageView.image = image
            let frame = CGRect(x: boardInitPoint.x, y: boardInitPoint.y, width: boardInitSize.width, height: boardInitSize.height)
            
            self.boardImageView.frame = resetBoardFrame(board: frame)
            let point = self.boardImageView.frame.origin
            ltCircle = UIView.circle(frame: CGRect(x: point.x, y: point.y, width: kBoardMarkSize, height: kBoardMarkSize))
            rtCircle = UIView.circle(frame: CGRect(x: frame.maxX, y: point.y, width: kBoardMarkSize, height: kBoardMarkSize))
            lbCircle = UIView.circle(frame: CGRect(x: point.x, y: frame.maxY, width: kBoardMarkSize, height: kBoardMarkSize))
            rbCircle = UIView.circle(frame: CGRect(x: frame.maxX, y: frame.maxY, width: kBoardMarkSize, height: kBoardMarkSize))
            
            addSubView(ltCircle!, tag: Marker.LeftTop)
            addSubView(rtCircle!, tag: Marker.RightTop)
            addSubView(lbCircle!, tag: Marker.LeftBottom)
            addSubView(rbCircle!, tag: Marker.RightBottom)
        }

        var isHidden = UserDefaults.standard.bool(forKey: kBoardMode)
        if let proprity = blackboardViewPriority {
            // 黒板表示優先モードが「Web」で、Webから取得した「isNeedBlackBoard」がfalaeの場合、黒板は非表示とする
            if (proprity == "Web") {
                if let showFlag = isNeedBlackBoard {
                    isHidden = !showFlag
                } else {
                    // Defaultは黒板を表示
                    isHidden = false
                }
            }
        }
        toggleBlackBoardMode(isHidden)
        
        // 監視を開始
        audioSession.addObserver(self, forKeyPath: "outputVolume", options: [ .new ], context: nil)
        
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?,
                               change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?) {
        if (currentVolumn > audioSession.outputVolume) {
            // Volumn Down
            cameraButton_TouchUpInside(Any.self)
        }
        currentVolumn = audioSession.outputVolume
    }

    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        cameraPreviewLayer?.position = CGPoint(x: self.cameraView.frame.width / 2, y: self.cameraView.frame.height / 2)
        cameraPreviewLayer?.bounds = cameraView.frame
        if let boardImageView = self.boardImageView {
            boardImageView.frame = resetBoardFrame(board: boardImageView.frame)
        }
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func addSubView(_ view: UIView, tag: Marker) {
        view.tag = tag.rawValue
        self.view.addSubview(view)
    }

    @objc
    func onOrientationDidChange(notification: NSNotification) {
        rotationCamera()
    }

    func rotationCamera() {
        // ここに回転時の処理
         var isLandscape = false
         let orientation = UIDevice.current.orientation
         if orientation == UIDeviceOrientation.landscapeLeft {
             isLandscape = true
             // なぜかLeft・Rightは逆の値を指定
             self.cameraPreviewLayer?.connection?.videoOrientation = AVCaptureVideoOrientation.landscapeRight
         } else if orientation == UIDeviceOrientation.landscapeRight {
             isLandscape = true
             self.cameraPreviewLayer?.connection?.videoOrientation = AVCaptureVideoOrientation.landscapeLeft
         } else if orientation == UIDeviceOrientation.portrait {
             isLandscape = false
             self.cameraPreviewLayer?.connection?.videoOrientation = AVCaptureVideoOrientation.portrait
         } else if orientation == UIDeviceOrientation.portraitUpsideDown {
             isLandscape = false
             self.cameraPreviewLayer?.connection?.videoOrientation = AVCaptureVideoOrientation.portraitUpsideDown
         } else {
             // UIDeviceOrientation.faceUp or UIDeviceOrientation.faceDown
             return
         }
         if isLandscape {
            print("Landscape")
            NSLayoutConstraint.deactivate(vViewConstraints)
            NSLayoutConstraint.activate(hViewConstraints)
        } else {
            print("Portrait")
            NSLayoutConstraint.deactivate(hViewConstraints)
            NSLayoutConstraint.activate(vViewConstraints)
        }
        self.boardImageView.frame = resetBoardFrame(board: self.boardImageView.frame)
    }

    // シャッターボタンが押された時のアクション
    @IBAction func cameraButton_TouchUpInside(_ sender: Any) {
        let settings = AVCapturePhotoSettings()
        let device = AVCaptureDevice.default(
            AVCaptureDevice.DeviceType.builtInWideAngleCamera,
            for: AVMediaType.video, // ビデオ入力
            position: AVCaptureDevice.Position.back)
        if !device!.hasFlash{
            settings.flashMode = .off
        } else {
            // フラッシュの設定
            switch flashModeIndex {
            case 1: // off
                settings.flashMode = .off
            case 2: // on
                settings.flashMode = .on
            default:
                settings.flashMode = .auto
            }
        }

        // カメラの手ぶれ補正
        settings.isAutoStillImageStabilizationEnabled = true
        // 撮影された画像をdelegateメソッドで処理
        self.photoOutput?.capturePhoto(with: settings, delegate: self as AVCapturePhotoCaptureDelegate)
    }

    @IBAction func backButton_TouchUpInside(_ sender: Any) {
        let back = BlackboardCamera();
        back.invoke(callbackId: self.callbackId, commandDelegate: self.commandDelegate, data: "Close", mode: self.blackboardViewPriority!)
        self.dismiss(animated: true, completion: nil)
        // ボリューム通知を解除
        audioSession.removeObserver(self, forKeyPath: "outputVolume")
    }

    @IBAction func flashButton_TouchUpInside(_ sender: Any) {
        flashModeIndex += 1
        if flashModeIndex > 2 {
            flashModeIndex = 0
        }
        UserDefaults.standard.set(flashModeIndex, forKey: kFlashMode)
        UserDefaults.standard.synchronize()
        changeFlashMode()
    }
    
    @IBAction func blackBoardMode_TouchUpInside(_ sender: Any) {
        let mode = !boardImageView.isHidden
        UserDefaults.standard.set(mode, forKey: kBoardMode)
        UserDefaults.standard.synchronize()
        toggleBlackBoardMode(mode)
        self.blackboardViewPriority = "App"
    }
    
    @IBAction func blackBoardEdit_TouchUpInside(_ sender: Any) {
        let back = BlackboardCamera();
        back.invoke(callbackId: self.callbackId, commandDelegate: self.commandDelegate, data: "Edit", mode: self.blackboardViewPriority!)
        self.dismiss(animated: true, completion: nil)
        // ボリューム通知を解除
        audioSession.removeObserver(self, forKeyPath: "outputVolume")
    }
    
    func toggleBlackBoardMode(_ mode: Bool) {
        ltCircle?.isHidden = mode
        rtCircle?.isHidden = mode
        lbCircle?.isHidden = mode
        rbCircle?.isHidden = mode
        boardImageView.isHidden = mode // Toggle Mode
        blackBoardModeText.text = mode ? "黒板ON" : "黒板OFF"
    }
    
    func cameraMarginPoint() -> CGPoint {
        let viewSize = self.view.frame.size
        let cameraSize = self.cameraView.frame.size
        return CGPoint(x: (viewSize.width - cameraSize.width) / 2, y: (viewSize.height - cameraSize.height) / 2)
    }

    func changeFlashMode() {
        switch flashModeIndex {
        case 1: // off
            self.flashImage.image = UIImage(named: "icn_flash_off.png")
            self.flashButton.setTitle("OFF", for: .normal)
        case 2: // on
            self.flashImage.image = UIImage(named: "icn_flash_on.png")
            self.flashButton.setTitle("ON", for: .normal)
        default: // auto
            self.flashImage.image = UIImage(named: "icn_flash_auto.png")
            self.flashButton.setTitle("自動", for: .normal)
        }
    }
    
    func checkInPointAvailable(target: CGPoint) -> Bool {
        var boardFrame = self.boardImageView.frame
        boardFrame.origin = boardFrame.origin.offsetBy(cameraViewMargin)
        return boardFrame.contains(target)
    }
    
    // left:top(1)   ---right:top(2)
    // left:bottom(3)---left:bottom(4)
    func checkPointType(target: CGPoint) -> Marker {
        let boardFrame = self.boardImageView.frame
        let margin = kBoardMarkSize / 2
        let inViewFrame = CGRect(x: boardFrame.origin.x + margin, y: boardFrame.origin.y + margin, width: boardFrame.size.width - margin, height: boardFrame.size.height - margin)
//        let boardOrigin = boardFrame.origin
//        boardFrame.origin = boardOrigin.offsetBy(cameraViewMargin)
        if (inViewFrame.contains(target)) {
            return Marker.Inside
        }
        if (ltCircle!.frame.contains(target)) {
            return Marker.LeftTop
        } else if (rtCircle!.frame.contains(target)) {
            return Marker.RightTop
        } else if (lbCircle!.frame.contains(target)) {
            return Marker.LeftBottom
        } else if (rbCircle!.frame.contains(target)) {
            return Marker.RightBottom
        }
        // 対象外
        return Marker.None
    }

    func resetBoardFrame(board: CGRect) -> CGRect {
        let cameraSize = self.cameraView.frame.size
        let offset = self.cameraView.frame.origin
        var boardSize = board.size
        var boardPoint = board.origin

        // board left
        if (boardPoint.x < offset.x) {
            boardPoint.x = offset.x
        // board right
        } else if (boardPoint.x + boardSize.width > cameraSize.width + offset.x) {
            boardPoint.x = offset.x + cameraSize.width - boardSize.width
        }
        // board top
        if (boardPoint.y < offset.y) {
            boardPoint.y = offset.y
        // board bottom
        } else if (boardPoint.y + boardSize.height > cameraSize.height + offset.y) {
            boardPoint.y =  offset.y + cameraSize.height - boardSize.height
        }
        
        // 横向きの場合、黒板の高さはマージンを適用したサイズより小さく設定
        var checkSize: CGFloat = 0
        if UIDevice.current.orientation.isLandscape {
            checkSize = cameraSize.height * 0.9
            if (boardSize.height > checkSize) {
                let aspect: CGFloat = boardSize.height / boardSize.width
                print("isLandscape:cameraSize=\(cameraSize), aspect=\(aspect)、 boardSize1=\(boardSize)")
                boardSize = CGSize(width: checkSize / aspect, height: checkSize)
            }
        } else {
            
            checkSize = cameraSize.width * 0.9
            if (boardSize.width > checkSize) {
                let aspect: CGFloat = boardSize.width / boardSize.height
                print("isPortrait:cameraSize=\(cameraSize), aspect=\(aspect)、 boardSize1=\(boardSize)")
                boardSize = CGSize(width: checkSize, height: checkSize / aspect)
            }
        }
        
        print("boardPoint=\(boardPoint), boardSize2=\(boardSize)")

        var newBoard = board
        newBoard.origin = boardPoint
        newBoard.size = boardSize

        self.ltCircle?.center = newBoard.origin
        self.rtCircle?.center = CGPoint(x: newBoard.maxX, y: newBoard.origin.y)
        self.lbCircle?.center = CGPoint(x: newBoard.origin.x, y: newBoard.maxY)
        self.rbCircle?.center = CGPoint(x: newBoard.maxX, y: newBoard.maxY)

        return newBoard
    }

    func touchesProcess(_ touches: Set<UITouch>) {
        guard self.boardImage != nil else { return }
        guard pointType != Marker.None else { return }

        if let touch = touches.first {
            let location = touch.location(in: self.view)
            if let locationInit = locationInit {
                let frame = self.boardImageView.frame
                let dx = location.x - locationInit.x
                let dy = location.y - locationInit.y
                var afterFrame = CGRect.zero
                switch pointType {
                case .LeftTop:
                    // 斜め移動のため片方の移動分のみ使う
                    let offsetFrame = frame.offsetBy(dx: dx, dy: dx)
                    afterFrame.origin = offsetFrame.origin // 左上移動
                    let width = checkMinWidth(currentBoardFrame.maxX - offsetFrame.minX)
                    // heightはwidthの増加分に合わせて設定
                    afterFrame.size = CGSize(width: width, height: currentBoardFrame.height / currentBoardFrame.width * width)
                case .RightTop:
                    let offsetFrame = frame.offsetBy(dx: dx, dy: -dx)
                    afterFrame.origin = CGPoint(x: currentBoardFrame.minX, y: offsetFrame.minY) // 右上移動
                    let width = checkMinWidth(offsetFrame.maxX - currentBoardFrame.minX)
                    afterFrame.size = CGSize(width: width, height: currentBoardFrame.height / currentBoardFrame.width * width)
                case .LeftBottom:
                    let offsetFrame = frame.offsetBy(dx: dx, dy: dx)
                    afterFrame.origin = CGPoint(x: offsetFrame.minX, y: frame.minY) // 左下移動
                    let width = checkMinWidth(currentBoardFrame.maxX - offsetFrame.minX)
                    afterFrame.size = CGSize(width: width, height: currentBoardFrame.height / currentBoardFrame.width * width)
                case .RightBottom:
                    let offsetFrame = frame.offsetBy(dx: dx, dy: dx)
                    let width = checkMinWidth(offsetFrame.maxX - currentBoardFrame.minX)
                    afterFrame.origin = frame.origin // 右下移動
                    afterFrame.size = CGSize(width: width, height: currentBoardFrame.height / currentBoardFrame.width * width)
                case .Inside:
                    afterFrame = frame.offsetBy(dx: dx, dy: dy)
                case .some(.None): return
                case .none: return
                }
                afterFrame = resetBoardFrame(board: afterFrame)
                self.boardImageView.frame = afterFrame
            }
            locationInit = location
        }
    }

    func checkMinWidth(_ width: CGFloat) -> CGFloat {
        let size = self.view.frame.size
        let minSize = min(size.width, size.height)
        let boardWidth = Float(minSize * kBoardMinScale)
        if (width < CGFloat(boardWidth)) {
            return CGFloat(boardWidth)
        }
        return width
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard self.boardImage != nil else { return }
        if let touch = touches.first {
            let location = touch.location(in: self.view)
            self.pointType = checkPointType(target: location)
            print("pointType=\(String(describing: pointType))")
            guard  pointType != Marker.None else {
                return
            }
            self.locationInit = location
            self.currentBoardFrame = self.boardImageView.frame
            print("touchesBegan：\(location)")
        }
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        touchesProcess(touches)
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        touchesProcess(touches)
        print("frame.origin=\(boardImageView.frame.origin), leftTop.origin=\(ltCircle!.center)")
        let boardPoint = boardImageView.frame.origin
        // UserDefaultに黒板の座標を保存しておく
        UserDefaults.standard.set(boardPoint.x, forKey: kBoardInitX)
        UserDefaults.standard.set(boardPoint.y, forKey: kBoardInitY)
        UserDefaults.standard.set(boardImageView.frame.width, forKey: kBoardInitWidth)
        UserDefaults.standard.synchronize()
    }

    // ジェスチャーイベント処理
    @objc func tapGesture(_ gestureRecognizer: UITapGestureRecognizer){
        print("tapGesture::\(String(describing: gestureRecognizer.view?.tag))")
    }

}

//MARK: AVCapturePhotoCaptureDelegateデリゲートメソッド
extension CameraViewController: AVCapturePhotoCaptureDelegate {
    // 撮影した画像データが生成されたときに呼び出されるデリゲートメソッド
    func photoOutput(_ output: AVCapturePhotoOutput, didFinishProcessingPhoto photo: AVCapturePhoto, error: Error?) {
        guard let imageData = photo.fileDataRepresentation() else {
            // TODO error
            return
        }
        // Data型をUIImageオブジェクトに変換
        guard let image = UIImage(data: imageData) else {
            // TODO error
            return
        }
        var newImage: UIImage?
        if UIDevice.current.orientation.isLandscape {
            newImage = UIImage(cgImage: image.cgImage!, scale: 1.0, orientation: .up)
        } else {
            newImage = image.fixedOrientation()
        }

        // 黒板合成
        var compositeImage: UIImage?
        if boardImageView.isHidden {
            compositeImage = newImage!
        } else {
            compositeImage = composite(image: newImage!)
        }

        let resizedImage = compositeImage!.resize()
        // 写真ライブラリに画像を保存 Debug用
        UIImageWriteToSavedPhotosAlbum(resizedImage!, nil,nil,nil)

        let data = compositeImage!.jpegData(compressionQuality: 1.0)
        if let jpegData = data {
//            let base64String = jpegData.base64EncodedString(options: .lineLength64Characters)
            let timestamp = NSDate().timeIntervalSince1970
            let filename = getDocumentsDirectory().appendingPathComponent("_\(timestamp).jpeg")
            try? jpegData.write(to: filename)
            let back = BlackboardCamera();
            back.invoke(callbackId: self.callbackId, commandDelegate: self.commandDelegate, data: filename.absoluteString, mode: self.blackboardViewPriority!)
            print("filename:::::::\(filename.absoluteString)")
            self.dismiss(animated: true, completion: nil)
            // ボリューム通知を解除
            audioSession.removeObserver(self, forKeyPath: "outputVolume")
        }

    }

    func getDocumentsDirectory() -> URL {
        let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
        return paths[0]
    }

    func composite(image: UIImage) -> UIImage? {
        UIGraphicsBeginImageContextWithOptions(image.size, false, 1.0)
        image.draw(in: CGRect(x: 0, y: 0, width: image.size.width, height: image.size.height))
        if let board = boardImage {
            var boardFrame = boardImageView.frame
            let scale =  image.size.width / cameraView.frame.size.width

            let boardPoint = boardFrame.origin

            let covertPoint = CGPoint(x: (boardPoint.x - cameraView.frame.origin.x) * scale, y: (boardPoint.y - cameraView.frame.origin.y) * scale)
            boardFrame.origin = covertPoint
            boardFrame.size = CGSize(width: boardFrame.size.width * scale, height: boardFrame.size.height * scale)
            board.draw(in: boardFrame)
        }

        let newImage = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        return newImage
    }
}

//MARK: カメラ設定メソッド
extension CameraViewController{
    // カメラの画質の設定
    func setupCaptureSession() {
        captureSession.sessionPreset = AVCaptureSession.Preset.photo
    }

    // デバイスの設定
    func setupDevice() {
        // カメラデバイスのプロパティ設定
        let deviceDiscoverySession = AVCaptureDevice.DiscoverySession(deviceTypes: [AVCaptureDevice.DeviceType.builtInWideAngleCamera], mediaType: AVMediaType.video, position: AVCaptureDevice.Position.unspecified)
        // プロパティの条件を満たしたカメラデバイスの取得
        let devices = deviceDiscoverySession.devices

        for device in devices {
            if device.position == AVCaptureDevice.Position.back {
                mainCamera = device
            } else if device.position == AVCaptureDevice.Position.front {
                innerCamera = device
            }
        }
        // 起動時のカメラを設定
        currentDevice = mainCamera
    }

    // 入出力データの設定
    func setupInputOutput() {
        do {
            // 指定したデバイスを使用するために入力を初期化
            let captureDeviceInput = try AVCaptureDeviceInput(device: currentDevice!)
            // 指定した入力をセッションに追加
            captureSession.addInput(captureDeviceInput)
            // 出力データを受け取るオブジェクトの作成
            photoOutput = AVCapturePhotoOutput()
            // 出力ファイルのフォーマットを指定
            photoOutput!.setPreparedPhotoSettingsArray([AVCapturePhotoSettings(format: [AVVideoCodecKey : AVVideoCodecType.jpeg])], completionHandler: nil)
            captureSession.addOutput(photoOutput!)
        } catch {
            print(error)
        }
    }

    // カメラのプレビューを表示するレイヤの設定
    func setupPreviewLayer() {
        // 指定したAVCaptureSessionでプレビューレイヤを初期化
        self.cameraPreviewLayer = AVCaptureVideoPreviewLayer(session: captureSession)
        // プレビューレイヤが、カメラのキャプチャーを縦横比を維持した状態で、表示するように設定
        self.cameraPreviewLayer?.videoGravity = AVLayerVideoGravity.resizeAspectFill
        // プレビューレイヤの表示の向きを設定
        self.cameraPreviewLayer?.connection?.videoOrientation = AVCaptureVideoOrientation.portrait
        self.cameraView.layer.addSublayer(self.cameraPreviewLayer!)
    }
    
    func setFromSavedData() {
        if let board = boardImage {
            let userDefault = UserDefaults.standard
            self.flashModeIndex = userDefault.integer(forKey: kFlashMode)
            let boardX = userDefault.float(forKey: kBoardInitX)
            let boardY = userDefault.float(forKey: kBoardInitY)
            var boardWidth = userDefault.float(forKey: kBoardInitWidth)
            var scale: Float = 1.0
            if (boardWidth == 0) {
                let size = self.view.frame.size
                let minSize = min(size.width, size.height)
                boardWidth = Float(minSize * kBoardBaseScale)
            }
            scale = Float(board.size.width) / boardWidth
            self.boardInitPoint = CGPoint(x: boardX.f, y: boardY.f)
            self.boardInitSize = CGSize(width: board.size.width / scale.f, height: board.size.height / scale.f)
        }
    }
    

//    // ボタンのスタイルを設定
//    func styleCaptureButton() {
//        cameraButton.layer.borderColor = UIColor.white.cgColor
//        cameraButton.layer.borderWidth = 5
//        cameraButton.clipsToBounds = true
//        cameraButton.layer.cornerRadius = min(cameraButton.frame.width, cameraButton.frame.height) / 2
//    }
}

extension UIImage {

    func fixedOrientation() -> UIImage {

        if imageOrientation == UIImage.Orientation.up {
            return self
        }

        var transform: CGAffineTransform = CGAffineTransform.identity

        switch imageOrientation {
        case .down, .downMirrored:
            transform = transform.translatedBy(x: size.width, y: size.height)
            transform = transform.rotated(by: CGFloat.pi)
            break
        case .left, .leftMirrored:
            transform = transform.translatedBy(x: size.width, y: 0)
            transform = transform.rotated(by: CGFloat.pi / 2.0)
            break
        case .right, .rightMirrored:
            transform = transform.translatedBy(x: 0, y: size.height)
            transform = transform.rotated(by: CGFloat.pi / -2.0)
            break
        case .up, .upMirrored:
            break
        @unknown default:
            break
        }
        switch imageOrientation {
        case .upMirrored, .downMirrored:
            transform.translatedBy(x: size.width, y: 0)
            transform.scaledBy(x: -1, y: 1)
            break
        case .leftMirrored, .rightMirrored:
            transform.translatedBy(x: size.height, y: 0)
            transform.scaledBy(x: -1, y: 1)
        case .up, .down, .left, .right:
            break
        @unknown default:
            break
        }

        let ctx: CGContext = CGContext(data: nil, width: Int(size.width), height: Int(size.height), bitsPerComponent: self.cgImage!.bitsPerComponent, bytesPerRow: 0, space: self.cgImage!.colorSpace!, bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)!
        ctx.concatenate(transform)
        switch imageOrientation {
        case .left, .leftMirrored, .right, .rightMirrored:
            ctx.draw(self.cgImage!, in: CGRect(x: 0, y: 0, width: size.height, height: size.width))
        default:
            ctx.draw(self.cgImage!, in: CGRect(x: 0, y: 0, width: size.width, height: size.height))
            break
        }
        return UIImage(cgImage: ctx.makeImage()!)
    }

    // 仕様：width x height = 100万画素以上
    func resize() -> UIImage? {
        var canvasSize: CGSize = CGSize.init()
        if (self.size.width > self.size.height) {
            // Landscape
            canvasSize.width = 1156
            canvasSize.height = 867
        } else {
            // Portrait
            canvasSize.width = 867
            canvasSize.height = 1156
        }
        // scaleを1.0にすることで指定サイズ通り作成される
        UIGraphicsBeginImageContextWithOptions(canvasSize, false, 1.0)
        defer { UIGraphicsEndImageContext() }
        draw(in: CGRect(origin: .zero, size: canvasSize))
        return UIGraphicsGetImageFromCurrentImageContext()!
    }
}

extension Float {
    public var f: CGFloat {
        return CGFloat(self)
    }
}

extension UIView {
    static func circle(frame: CGRect) -> UIView {
        let circle = UIView(frame: frame)
        circle.center = frame.origin
        circle.layer.cornerRadius = frame.size.width / 2
        circle.layer.borderColor = UIColor.red.cgColor
        circle.clipsToBounds = true
        circle.backgroundColor = UIColor.white
        return circle
    }
}

extension CGPoint {
  /// Retuns the point which is an offset of an existing point.
  ///
  /// - Parameters:
  ///   - dx: The x-coordinate offset to apply.
  ///   - dy: The y-coordinate offset to apply.
  ///
  /// - Returns:
  ///   A new point which is an offset of an existing point.
  func offsetBy(_ d: CGPoint) -> CGPoint {
    return CGPoint(x: x - d.x, y: y - d.y)
  }
}

