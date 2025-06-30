import { TransactionProcessor, Transaction } from "../../src/core/TransactionProcessor";
import { createHash } from "crypto";

describe("TransactionProcessor", () => {
    let processor: TransactionProcessor;
    let sampleTransaction: Transaction;

    beforeEach(() => {
        processor = new TransactionProcessor();
        sampleTransaction = {
            sender: "0xabc",
            receiver: "0xdef",
            amount: 100,
            timestamp: 1234567890,
            signature: "0x123"
        };
        // Calculate hash
        const txCopy = { ...sampleTransaction };
        const txStr = JSON.stringify(txCopy);
        sampleTransaction.hash = createHash("sha256").update(txStr).digest("hex");
    });

    describe("initialize", () => {
        it("should initialize successfully", async () => {
            const result = await processor.initialize();
            expect(result).toBe(true);
            expect(processor.getLastError()).toBe("");
        });
    });

    describe("shutdown", () => {
        it("should shutdown successfully", async () => {
            await processor.initialize();
            const result = await processor.shutdown();
            expect(result).toBe(true);
            expect(await processor.getTransactionPool()).toEqual([]);
        });
    });

    describe("processTransaction", () => {
        beforeEach(async () => {
            await processor.initialize();
        });

        it("should process a valid transaction", async () => {
            const transactionData = Buffer.from(JSON.stringify(sampleTransaction));
            const result = await processor.processTransaction(transactionData);
            expect(result).toBe(true);
            expect(await processor.getTransaction(sampleTransaction.hash)).toEqual(sampleTransaction);
        });

        it("should reject an invalid transaction", async () => {
            const invalidTransaction = {
                hash: "0x123",
                sender: "0xabc"
                // Missing required fields
            };
            const transactionData = Buffer.from(JSON.stringify(invalidTransaction));
            const result = await processor.processTransaction(transactionData);
            expect(result).toBe(false);
            expect(processor.getLastError()).toContain("Missing required field");
        });

        it("should reject a duplicate transaction", async () => {
            const transactionData = Buffer.from(JSON.stringify(sampleTransaction));
            await processor.processTransaction(transactionData);
            const result = await processor.processTransaction(transactionData);
            expect(result).toBe(false);
            expect(processor.getLastError()).toContain("already processed");
        });

        it("should reject a transaction with invalid hash", async () => {
            const transaction = {
                hash: "0x123", // Invalid hash
                sender: "0xabc",
                receiver: "0xdef",
                amount: 100,
                timestamp: 1234567890,
                signature: "0x456"
            };
            const transactionData = Buffer.from(JSON.stringify(transaction));
            const result = await processor.processTransaction(transactionData);
            expect(result).toBe(false);
            expect(processor.getLastError()).toContain("Invalid transaction hash");
        });
    });

    describe("getTransaction", () => {
        beforeEach(async () => {
            await processor.initialize();
            const transactionData = Buffer.from(JSON.stringify(sampleTransaction));
            await processor.processTransaction(transactionData);
        });

        it("should get a transaction by hash", async () => {
            const transaction = await processor.getTransaction(sampleTransaction.hash);
            expect(transaction).toEqual(sampleTransaction);
        });

        it("should return null for non-existent transaction", async () => {
            const transaction = await processor.getTransaction("non-existent");
            expect(transaction).toBeNull();
        });
    });

    describe("getTransactionPool", () => {
        beforeEach(async () => {
            await processor.initialize();
            const transactionData = Buffer.from(JSON.stringify(sampleTransaction));
            await processor.processTransaction(transactionData);
        });

        it("should get all transactions in the pool", async () => {
            const pool = await processor.getTransactionPool();
            expect(pool).toHaveLength(1);
            expect(pool[0]).toEqual(sampleTransaction);
        });
    });

    describe("markTransactionProcessed", () => {
        beforeEach(async () => {
            await processor.initialize();
            const transactionData = Buffer.from(JSON.stringify(sampleTransaction));
            await processor.processTransaction(transactionData);
        });

        it("should mark a transaction as processed", async () => {
            const result = await processor.markTransactionProcessed(sampleTransaction.hash);
            expect(result).toBe(true);
            expect(await processor.getTransactionPool()).toHaveLength(0);
            expect(await processor.getTransaction(sampleTransaction.hash)).toEqual(sampleTransaction);
        });

        it("should fail for non-existent transaction", async () => {
            const result = await processor.markTransactionProcessed("non-existent");
            expect(result).toBe(false);
            expect(processor.getLastError()).toContain("not found in pool");
        });
    });

    describe("error handling", () => {
        it("should handle errors when not initialized", async () => {
            const result = await processor.processTransaction(Buffer.from("{}"));
            expect(result).toBe(false);
            expect(processor.getLastError()).toContain("not initialized");
        });

        it("should clear error message", async () => {
            await processor.processTransaction(Buffer.from("{}"));
            expect(processor.getLastError()).not.toBe("");
            processor.clearLastError();
            expect(processor.getLastError()).toBe("");
        });
    });
}); 