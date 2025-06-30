const EventEmitter = require("events");
const crypto = require("crypto");

class Transaction {
  constructor(options = {}) {
    this.version = options.version || 1;
    this.inputs = options.inputs || [];
    this.outputs = options.outputs || [];
    this.locktime = options.locktime || 0;
    this.timestamp = Date.now();
    this.fee = options.fee || 0;
    this.status = "pending";
    this.txid = null;
    this.blockHeight = null;
    this.confirmations = 0;
  }

  addInput(utxo) {
    this.inputs.push({
      txid: utxo.txid,
      vout: utxo.vout,
      scriptSig: utxo.scriptSig,
      sequence: 0xffffffff,
    });
  }

  addOutput(address, amount) {
    this.outputs.push({
      address,
      amount,
      scriptPubKey: this.createScriptPubKey(address),
    });
  }

  createScriptPubKey(address) {
    // In a real implementation, this would create the proper script
    return `OP_DUP OP_HASH160 ${address} OP_EQUALVERIFY OP_CHECKSIG`;
  }

  calculateFee() {
    const inputAmount = this.inputs.reduce(
      (sum, input) => sum + input.amount,
      0,
    );
    const outputAmount = this.outputs.reduce(
      (sum, output) => sum + output.amount,
      0,
    );
    this.fee = inputAmount - outputAmount;
    return this.fee;
  }

  toJSON() {
    return {
      version: this.version,
      inputs: this.inputs,
      outputs: this.outputs,
      locktime: this.locktime,
      timestamp: this.timestamp,
      fee: this.fee,
      status: this.status,
      txid: this.txid,
      blockHeight: this.blockHeight,
      confirmations: this.confirmations,
    };
  }
}

class TransactionManager extends EventEmitter {
  constructor() {
    super();
    this.initialized = false;
    this.transactions = new Map();
    this.config = {
      minFee: 0.0001,
      maxFee: 1.0,
      defaultFee: 0.001,
      maxInputs: 100,
      maxOutputs: 100,
      maxAmount: 1000000,
    };
  }

  initialize() {
    if (this.initialized) {
      throw new Error("Transaction Manager is already initialized");
    }
    this.initialized = true;
    this.emit("initialized");
  }

  isInitialized() {
    return this.initialized;
  }

  createTransaction(options = {}) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before creating transaction",
      );
    }

    const transaction = new Transaction(options);
    this.transactions.set(transaction.txid, transaction);
    this.emit("transactionCreated", { transaction });
    return transaction;
  }

  validateTransaction(transaction) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before validating transaction",
      );
    }

    // Basic validation
    if (!transaction.inputs || transaction.inputs.length === 0) {
      throw new Error("Transaction must have at least one input");
    }

    if (!transaction.outputs || transaction.outputs.length === 0) {
      throw new Error("Transaction must have at least one output");
    }

    if (transaction.inputs.length > this.config.maxInputs) {
      throw new Error(
        `Transaction exceeds maximum input count of ${this.config.maxInputs}`,
      );
    }

    if (transaction.outputs.length > this.config.maxOutputs) {
      throw new Error(
        `Transaction exceeds maximum output count of ${this.config.maxOutputs}`,
      );
    }

    // Amount validation
    const totalOutput = transaction.outputs.reduce(
      (sum, output) => sum + output.amount,
      0,
    );
    if (totalOutput > this.config.maxAmount) {
      throw new Error(
        `Transaction amount exceeds maximum of ${this.config.maxAmount}`,
      );
    }

    // Fee validation
    const fee = transaction.calculateFee();
    if (fee < this.config.minFee) {
      throw new Error(`Transaction fee below minimum of ${this.config.minFee}`);
    }
    if (fee > this.config.maxFee) {
      throw new Error(`Transaction fee above maximum of ${this.config.maxFee}`);
    }

    this.emit("transactionValidated", { transaction });
    return true;
  }

  signTransaction(transaction, privateKey) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before signing transaction",
      );
    }

    // In a real implementation, this would sign each input
    // using the provided private key
    transaction.inputs.forEach((input) => {
      input.scriptSig = this.createScriptSig(input, privateKey);
    });

    this.emit("transactionSigned", { transaction });
    return transaction;
  }

  createScriptSig(input, privateKey) {
    // In a real implementation, this would create the proper signature script
    return `SIGNATURE ${privateKey}`;
  }

  broadcastTransaction(transaction) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before broadcasting transaction",
      );
    }

    // In a real implementation, this would send the transaction to the network
    transaction.status = "broadcast";
    this.emit("transactionBroadcast", { transaction });
    return transaction;
  }

  getTransaction(txid) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before getting transaction",
      );
    }

    const transaction = this.transactions.get(txid);
    if (!transaction) {
      throw new Error(`Transaction ${txid} not found`);
    }

    return transaction;
  }

  updateTransactionStatus(txid, status, blockHeight = null) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before updating transaction status",
      );
    }

    const transaction = this.getTransaction(txid);
    transaction.status = status;
    if (blockHeight) {
      transaction.blockHeight = blockHeight;
      transaction.confirmations = this.calculateConfirmations(blockHeight);
    }

    this.emit("transactionStatusUpdated", { transaction });
    return transaction;
  }

  calculateConfirmations(blockHeight) {
    // In a real implementation, this would calculate confirmations
    // based on the current blockchain height
    return 0;
  }

  estimateFee(inputs, outputs) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before estimating fee",
      );
    }

    // In a real implementation, this would calculate the fee
    // based on the transaction size and current network conditions
    return this.config.defaultFee;
  }

  getUnspentOutputs(address) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before getting unspent outputs",
      );
    }

    // In a real implementation, this would query the blockchain
    // for unspent transaction outputs
    return [];
  }

  listTransactions(status = null) {
    if (!this.initialized) {
      throw new Error(
        "Transaction Manager must be initialized before listing transactions",
      );
    }

    let transactions = Array.from(this.transactions.values());
    if (status) {
      transactions = transactions.filter((tx) => tx.status === status);
    }

    return transactions;
  }
}

module.exports = TransactionManager;
