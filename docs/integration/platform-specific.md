# Platform-Specific Guides

This document provides platform-specific integration guides for Satox SDK.

## Core SDK (C++)

### Prerequisites
- C++17 or later
- CMake 3.14+
- OpenSSL 1.1+
- PostgreSQL development libraries
- Quantum-resistant cryptography libraries

### Installation

#### Ubuntu/Debian
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake libssl-dev libpq-dev

# Clone and build
git clone https://github.com/satox/satox-sdk.git
cd satox-sdk
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

#### macOS
```bash
# Install dependencies
brew install cmake openssl postgresql

# Clone and build
git clone https://github.com/satox/satox-sdk.git
cd satox-sdk
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
sudo make install
```

#### Windows
```bash
# Install Visual Studio 2019 or later
# Install CMake and vcpkg

# Install dependencies
vcpkg install openssl postgresql

# Clone and build
git clone https://github.com/satox/satox-sdk.git
cd satox-sdk
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Usage Example
```cpp
#include <satox/sdk.hpp>
#include <iostream>

int main() {
    // Initialize SDK
    SatoxSDK sdk;
    Config config;
    config.network = "mainnet";
    config.quantum_enabled = true;
    config.database_url = "postgresql://user:pass@localhost/satox";
    
    if (!sdk.initialize(config)) {
        std::cerr << "Failed to initialize SDK" << std::endl;
        return 1;
    }
    
    // Create wallet
    Wallet wallet = sdk.createWallet();
    std::cout << "Wallet address: " << wallet.getAddress() << std::endl;
    
    // Create asset
    Asset asset = sdk.createAsset("My Token", "MTK", 1000000);
    std::cout << "Asset ID: " << asset.getId() << std::endl;
    
    return 0;
}
```

### CMake Integration
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(MySatoxApp)

find_package(SatoxSDK REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp SatoxSDK::satox)
```

## Mobile SDK

### iOS (Swift)

#### Prerequisites
- Xcode 12.0+
- iOS 13.0+
- Swift 5.0+

#### Installation

##### Swift Package Manager
```swift
// Package.swift
dependencies: [
    .package(url: "https://github.com/satox/satox-mobile-sdk.git", from: "1.0.0")
]
```

##### CocoaPods
```ruby
# Podfile
pod 'SatoxSDK', '~> 1.0'
```

#### Usage Example
```swift
import SatoxSDK

class ViewController: UIViewController {
    var sdk: SatoxSDK!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Initialize SDK
        let config = Config()
        config.network = "mainnet"
        config.quantumEnabled = true
        
        sdk = SatoxSDK()
        sdk.initialize(config: config) { success in
            if success {
                print("SDK initialized successfully")
                self.createWallet()
            } else {
                print("Failed to initialize SDK")
            }
        }
    }
    
    func createWallet() {
        sdk.createWallet { wallet in
            if let wallet = wallet {
                print("Wallet created: \(wallet.address)")
                self.createAsset()
            }
        }
    }
    
    func createAsset() {
        sdk.createAsset(name: "My Token", symbol: "MTK", supply: 1000000) { asset in
            if let asset = asset {
                print("Asset created: \(asset.id)")
            }
        }
    }
}
```

### Android (Kotlin)

#### Prerequisites
- Android Studio 4.0+
- Android API 21+
- Kotlin 1.4+

#### Installation

##### Gradle
```gradle
// build.gradle
dependencies {
    implementation 'com.satox:sdk:1.0.0'
}
```

#### Usage Example
```kotlin
import com.satox.sdk.*

class MainActivity : AppCompatActivity() {
    private lateinit var sdk: SatoxSDK
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        // Initialize SDK
        val config = Config().apply {
            network = "mainnet"
            quantumEnabled = true
        }
        
        sdk = SatoxSDK()
        sdk.initialize(config) { success ->
            if (success) {
                Log.d("SatoxSDK", "SDK initialized successfully")
                createWallet()
            } else {
                Log.e("SatoxSDK", "Failed to initialize SDK")
            }
        }
    }
    
    private fun createWallet() {
        sdk.createWallet { wallet ->
            wallet?.let {
                Log.d("SatoxSDK", "Wallet created: ${it.address}")
                createAsset()
            }
        }
    }
    
    private fun createAsset() {
        sdk.createAsset("My Token", "MTK", 1000000L) { asset ->
            asset?.let {
                Log.d("SatoxSDK", "Asset created: ${it.id}")
            }
        }
    }
}
```

### React Native

#### Prerequisites
- Node.js 14+
- React Native 0.63+
- Android Studio / Xcode

#### Installation
```bash
npm install @satox/react-native-sdk
# or
yarn add @satox/react-native-sdk
```

#### Usage Example
```javascript
import SatoxSDK from '@satox/react-native-sdk';

const App = () => {
  const [sdk, setSdk] = useState(null);
  const [wallet, setWallet] = useState(null);

  useEffect(() => {
    initializeSDK();
  }, []);

  const initializeSDK = async () => {
    try {
      const config = {
        network: 'mainnet',
        quantumEnabled: true,
      };
      
      const sdkInstance = new SatoxSDK();
      await sdkInstance.initialize(config);
      setSdk(sdkInstance);
      
      console.log('SDK initialized successfully');
    } catch (error) {
      console.error('Failed to initialize SDK:', error);
    }
  };

  const createWallet = async () => {
    try {
      const newWallet = await sdk.createWallet();
      setWallet(newWallet);
      console.log('Wallet created:', newWallet.address);
    } catch (error) {
      console.error('Failed to create wallet:', error);
    }
  };

  const createAsset = async () => {
    try {
      const asset = await sdk.createAsset('My Token', 'MTK', 1000000);
      console.log('Asset created:', asset.id);
    } catch (error) {
      console.error('Failed to create asset:', error);
    }
  };

  return (
    <View style={styles.container}>
      <Button title="Create Wallet" onPress={createWallet} />
      <Button title="Create Asset" onPress={createAsset} />
    </View>
  );
};
```

### Flutter

#### Prerequisites
- Flutter 2.0+
- Dart 2.12+
- Android Studio / Xcode

#### Installation
```yaml
# pubspec.yaml
dependencies:
  satox_sdk: ^1.0.0
```

#### Usage Example
```dart
import 'package:satox_sdk/satox_sdk.dart';

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  SatoxSDK? sdk;
  Wallet? wallet;

  @override
  void initState() {
    super.initState();
    initializeSDK();
  }

  Future<void> initializeSDK() async {
    try {
      final config = Config(
        network: 'mainnet',
        quantumEnabled: true,
      );
      
      sdk = SatoxSDK();
      await sdk!.initialize(config);
      
      print('SDK initialized successfully');
    } catch (e) {
      print('Failed to initialize SDK: $e');
    }
  }

  Future<void> createWallet() async {
    try {
      wallet = await sdk!.createWallet();
      print('Wallet created: ${wallet!.address}');
    } catch (e) {
      print('Failed to create wallet: $e');
    }
  }

  Future<void> createAsset() async {
    try {
      final asset = await sdk!.createAsset('My Token', 'MTK', 1000000);
      print('Asset created: ${asset.id}');
    } catch (e) {
      print('Failed to create asset: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text('Satox SDK Demo')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ElevatedButton(
                onPressed: createWallet,
                child: Text('Create Wallet'),
              ),
              ElevatedButton(
                onPressed: createAsset,
                child: Text('Create Asset'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
```

## Game SDK

### Unity

#### Prerequisites
- Unity 2020.3 LTS or later
- .NET 4.x or .NET Standard 2.1

#### Installation
1. Download the Unity package from the releases page
2. Import the package into your Unity project
3. Add the SDK to your project's assembly references

#### Usage Example
```csharp
using Satox.GameSDK;
using UnityEngine;

public class SatoxGameManager : MonoBehaviour
{
    private SatoxGameSDK sdk;
    private PlayerWallet playerWallet;

    void Start()
    {
        InitializeSDK();
    }

    async void InitializeSDK()
    {
        try
        {
            var config = new Config
            {
                Network = "mainnet",
                QuantumEnabled = true
            };

            sdk = new SatoxGameSDK();
            await sdk.Initialize(config);
            
            Debug.Log("SDK initialized successfully");
            CreatePlayerWallet();
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Failed to initialize SDK: {e.Message}");
        }
    }

    async void CreatePlayerWallet()
    {
        try
        {
            string playerId = SystemInfo.deviceUniqueIdentifier;
            playerWallet = await sdk.CreatePlayerWallet(playerId);
            
            Debug.Log($"Player wallet created: {playerWallet.Address}");
            CreateGameAsset();
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Failed to create wallet: {e.Message}");
        }
    }

    async void CreateGameAsset()
    {
        try
        {
            var asset = await sdk.CreateGameAsset("Game Token", "GT", 1000000);
            Debug.Log($"Game asset created: {asset.Id}");
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Failed to create asset: {e.Message}");
        }
    }

    async void TransferAsset(string recipientAddress, string assetId, long amount)
    {
        try
        {
            var transaction = await sdk.TransferAsset(
                playerWallet, 
                recipientAddress, 
                assetId, 
                amount
            );
            
            Debug.Log($"Asset transferred: {transaction.Id}");
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Failed to transfer asset: {e.Message}");
        }
    }
}
```

### Unreal Engine

#### Prerequisites
- Unreal Engine 4.27+ or 5.0+
- Visual Studio 2019+ (Windows)
- Xcode 12+ (macOS)

#### Installation
1. Download the Unreal Engine plugin
2. Extract to your project's Plugins folder
3. Enable the plugin in your project settings

#### Usage Example
```cpp
#include "SatoxGameSDK.h"
#include "Engine/Engine.h"

ASatoxGameManager::ASatoxGameManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASatoxGameManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeSDK();
}

void ASatoxGameManager::InitializeSDK()
{
    FConfig Config;
    Config.Network = TEXT("mainnet");
    Config.QuantumEnabled = true;
    
    SDK = NewObject<USatoxGameSDK>();
    SDK->Initialize(Config, FSDKInitializeComplete::CreateUObject(this, &ASatoxGameManager::OnSDKInitialized));
}

void ASatoxGameManager::OnSDKInitialized(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SDK initialized successfully"));
        CreatePlayerWallet();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize SDK"));
    }
}

void ASatoxGameManager::CreatePlayerWallet()
{
    FString PlayerId = FPlatformMisc::GetDeviceId();
    SDK->CreatePlayerWallet(PlayerId, FWalletCreated::CreateUObject(this, &ASatoxGameManager::OnWalletCreated));
}

void ASatoxGameManager::OnWalletCreated(UPlayerWallet* Wallet)
{
    if (Wallet)
    {
        PlayerWallet = Wallet;
        UE_LOG(LogTemp, Log, TEXT("Player wallet created: %s"), *Wallet->GetAddress());
        CreateGameAsset();
    }
}

void ASatoxGameManager::CreateGameAsset()
{
    SDK->CreateGameAsset(TEXT("Game Token"), TEXT("GT"), 1000000, 
        FAssetCreated::CreateUObject(this, &ASatoxGameManager::OnAssetCreated));
}

void ASatoxGameManager::OnAssetCreated(UGameAsset* Asset)
{
    if (Asset)
    {
        UE_LOG(LogTemp, Log, TEXT("Game asset created: %s"), *Asset->GetId());
    }
}
```

## Platform-Specific Considerations

### Security
- **iOS**: Use Keychain for secure storage
- **Android**: Use Keystore for secure storage
- **Unity**: Use Unity's secure storage APIs
- **Unreal**: Use platform-specific secure storage

### Performance
- **Mobile**: Implement background processing
- **Games**: Use async operations for blockchain calls
- **Desktop**: Use multi-threading for heavy operations

### Offline Support
- **Mobile**: Cache wallet data locally
- **Games**: Support offline asset management
- **Desktop**: Sync when connection restored

## Resources
- [Integration Overview](overview.md)
- [API Reference](api-reference.md)
- [Examples](examples.md)
- [Troubleshooting](../troubleshooting/common-issues.md) 