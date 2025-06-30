package com.satox;

import com.satox.asset.Asset;
import com.satox.asset.AssetManager;
import com.satox.asset.AssetFactory;
import com.satox.asset.AssetValidator;
import com.satox.asset.AssetStorage;
import com.satox.quantum.SatoxException;
import com.satox.ipfs.IPFSManager;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

public class SatoxSDKTest {
    private SatoxSDK sdk;
    private AssetManager assetManager;
    private IPFSManager ipfsManager;

    @BeforeEach
    public void setUp() throws Exception {
        sdk = SatoxSDK.getInstance();
        sdk.initialize();
        assetManager = sdk.getAssetManager();
        ipfsManager = sdk.getIPFSManager();
        assetManager.initialize();
        AssetFactory.getInstance().initialize();
        AssetValidator.getInstance().initialize();
        AssetStorage.getInstance().initialize();
    }

    @AfterEach
    public void tearDown() throws Exception {
        AssetStorage.getInstance().shutdown();
        AssetValidator.getInstance().shutdown();
        AssetFactory.getInstance().shutdown();
        assetManager.shutdown();
        sdk.shutdown();
    }

    @Test
    public void testSDKInitialization() {
        assertNotNull(sdk);
        assertNotNull(assetManager);
        assertNotNull(ipfsManager);
    }

    @Test
    public void testAssetCreation() throws SatoxException {
        String name = "Test Asset";
        String description = "Test Description";
        double value = 100.0;

        Asset asset = assetManager.createAsset(name, description, value);
        assertNotNull(asset);
        assertEquals(name, asset.getName());
        assertEquals(description, asset.getDescription());
        assertEquals(value, asset.getValue());
    }

    @Test
    public void testIPFSFileOperations() throws SatoxException {
        String filename = "test.txt";
        byte[] content = "Hello, World!".getBytes();

        String hash = ipfsManager.addFile(filename, content);
        assertNotNull(hash);

        byte[] retrievedContent = ipfsManager.getFile(hash);
        assertNotNull(retrievedContent);
        assertArrayEquals(content, retrievedContent);
    }
} 