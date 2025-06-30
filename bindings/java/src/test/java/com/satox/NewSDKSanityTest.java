package com.satox;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

public class NewSDKSanityTest {
    @Test
    public void testSDKLoadsAndManagersAccessible() {
        SatoxSDK sdk = SatoxSDK.getInstance();
        sdk.initialize();
        assertNotNull(sdk.getAssetManager());
        assertNotNull(sdk.getIPFSManager());
        assertNotNull(sdk.getTransactionManager());
        sdk.shutdown();
    }
} 