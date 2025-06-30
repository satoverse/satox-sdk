import { NetworkManager } from "../../src/core/NetworkManager";
import axios from "axios";

jest.mock("axios");
const mockedAxios = axios as jest.Mocked<typeof axios>;

describe("NetworkManager", () => {
    let manager: NetworkManager;

    beforeEach(() => {
        manager = new NetworkManager();
        jest.clearAllMocks();
    });

    describe("initialize", () => {
        it("should initialize successfully", async () => {
            const result = await manager.initialize();
            expect(result).toBe(true);
            expect(manager.getLastError()).toBe("");
        });
    });

    describe("shutdown", () => {
        it("should shutdown successfully", async () => {
            await manager.initialize();
            const result = await manager.shutdown();
            expect(result).toBe(true);
            expect(manager.getPeers()).toHaveLength(0);
        });
    });

    describe("addPeer", () => {
        beforeEach(async () => {
            await manager.initialize();
        });

        it("should add a peer successfully", async () => {
            const result = await manager.addPeer("peer1", "http://localhost:8000");
            expect(result).toBe(true);
            const peer = manager.getPeer("peer1");
            expect(peer).not.toBeNull();
            expect(peer?.address).toBe("http://localhost:8000");
            expect(peer?.connected).toBe(false);
        });

        it("should reject duplicate peer", async () => {
            await manager.addPeer("peer1", "http://localhost:8000");
            const result = await manager.addPeer("peer1", "http://localhost:8001");
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("already exists");
        });
    });

    describe("removePeer", () => {
        beforeEach(async () => {
            await manager.initialize();
            await manager.addPeer("peer1", "http://localhost:8000");
        });

        it("should remove a peer successfully", async () => {
            const result = await manager.removePeer("peer1");
            expect(result).toBe(true);
            expect(manager.getPeer("peer1")).toBeNull();
        });

        it("should fail for non-existent peer", async () => {
            const result = await manager.removePeer("nonexistent");
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("not found");
        });
    });

    describe("connectToPeer", () => {
        beforeEach(async () => {
            await manager.initialize();
            await manager.addPeer("peer1", "http://localhost:8000");
        });

        it("should connect to peer successfully", async () => {
            mockedAxios.get.mockResolvedValueOnce({ status: 200 });
            const result = await manager.connectToPeer("peer1");
            expect(result).toBe(true);
            const peer = manager.getPeer("peer1");
            expect(peer?.connected).toBe(true);
        });

        it("should fail for non-existent peer", async () => {
            const result = await manager.connectToPeer("nonexistent");
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("not found");
        });

        it("should handle connection failure", async () => {
            mockedAxios.get.mockRejectedValueOnce(new Error("Connection failed"));
            const result = await manager.connectToPeer("peer1");
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("Failed to connect");
        });
    });

    describe("sendMessage", () => {
        beforeEach(async () => {
            await manager.initialize();
            await manager.addPeer("peer1", "http://localhost:8000");
            mockedAxios.get.mockResolvedValueOnce({ status: 200 });
            await manager.connectToPeer("peer1");
        });

        it("should send message successfully", async () => {
            const message = { type: "test", data: "hello" };
            mockedAxios.post.mockResolvedValueOnce({ status: 200 });
            const result = await manager.sendMessage("peer1", message);
            expect(result).toBe(true);
        });

        it("should fail for non-existent peer", async () => {
            const message = { type: "test", data: "hello" };
            const result = await manager.sendMessage("nonexistent", message);
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("not found");
        });

        it("should fail for unconnected peer", async () => {
            await manager.addPeer("peer2", "http://localhost:8001");
            const message = { type: "test", data: "hello" };
            const result = await manager.sendMessage("peer2", message);
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("not connected");
        });

        it("should handle send failure", async () => {
            const message = { type: "test", data: "hello" };
            mockedAxios.post.mockRejectedValueOnce(new Error("Send failed"));
            const result = await manager.sendMessage("peer1", message);
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("Failed to send");
        });
    });

    describe("getPeers", () => {
        beforeEach(async () => {
            await manager.initialize();
            await manager.addPeer("peer1", "http://localhost:8000");
            await manager.addPeer("peer2", "http://localhost:8001");
        });

        it("should get all peers", () => {
            const peers = manager.getPeers();
            expect(peers).toHaveLength(2);
            expect(peers.some(p => p.id === "peer1")).toBe(true);
            expect(peers.some(p => p.id === "peer2")).toBe(true);
        });
    });

    describe("getPeer", () => {
        beforeEach(async () => {
            await manager.initialize();
            await manager.addPeer("peer1", "http://localhost:8000");
        });

        it("should get a specific peer", () => {
            const peer = manager.getPeer("peer1");
            expect(peer).not.toBeNull();
            expect(peer?.id).toBe("peer1");
            expect(peer?.address).toBe("http://localhost:8000");
        });

        it("should return null for non-existent peer", () => {
            const peer = manager.getPeer("nonexistent");
            expect(peer).toBeNull();
        });
    });

    describe("error handling", () => {
        it("should handle errors when not initialized", async () => {
            const result = await manager.addPeer("peer1", "http://localhost:8000");
            expect(result).toBe(false);
            expect(manager.getLastError()).toContain("not initialized");
        });

        it("should clear error message", async () => {
            await manager.addPeer("peer1", "http://localhost:8000");
            expect(manager.getLastError()).not.toBe("");
            manager.clearLastError();
            expect(manager.getLastError()).toBe("");
        });
    });
}); 