const EventEmitter = require("events");
const crypto = require("crypto");
const bip39 = require("bip39");
const { BIP32Factory } = require("bip32");
const ecc = require("tiny-secp256k1");
const bip32 = BIP32Factory(ecc);

class Wallet {
  constructor(name, mnemonic, password) {
    this.name = name;
    this.mnemonic = mnemonic;
    this.password = password;
    this.addresses = new Map();
    this.balance = 0;
    this.transactions = [];
    this.createdAt = Date.now();
    this.lastUsed = Date.now();
  }

  generateAddress(index) {
    const seed = bip39.mnemonicToSeedSync(this.mnemonic);
    const root = bip32.fromSeed(seed);
    const path = `m/44'/9007'/0'/0/${index}`; // Using Satoxcoin's SLIP-44: 9007
    const child = root.derivePath(path);
    const address = this.deriveAddress(child.publicKey);

    this.addresses.set(index, {
      address,
      publicKey: child.publicKey.toString("hex"),
      privateKey: child.privateKey.toString("hex"),
      index,
      balance: 0,
    });

    return address;
  }

  deriveAddress(publicKey) {
    const hash = crypto.createHash("sha256").update(publicKey).digest();
    const ripemd160 = crypto.createHash("ripemd160").update(hash).digest();
    return `S${ripemd160.toString("base58")}`; // Satoxcoin address prefix
  }

  getAddresses() {
    return Array.from(this.addresses.values());
  }

  getAddress(index) {
    return this.addresses.get(index);
  }

  updateBalance(address, balance) {
    const addr = this.addresses.get(address);
    if (addr) {
      addr.balance = balance;
      this.updateTotalBalance();
    }
  }

  updateTotalBalance() {
    this.balance = Array.from(this.addresses.values()).reduce(
      (total, addr) => total + addr.balance,
      0,
    );
  }

  addTransaction(tx) {
    this.transactions.push(tx);
    this.lastUsed = Date.now();
  }

  getTransactions() {
    return this.transactions;
  }

  toJSON() {
    return {
      name: this.name,
      addresses: this.getAddresses(),
      balance: this.balance,
      createdAt: this.createdAt,
      lastUsed: this.lastUsed,
    };
  }
}

class WalletManager extends EventEmitter {
  constructor() {
    super();
    this.initialized = false;
    this.wallets = new Map();
    this.currentWallet = null;
    this.config = {
      defaultAddressCount: 10,
      addressGapLimit: 20,
      minPasswordLength: 8,
    };
  }

  initialize() {
    if (this.initialized) {
      throw new Error("Wallet Manager is already initialized");
    }
    this.initialized = true;
    this.emit("initialized");
  }

  isInitialized() {
    return this.initialized;
  }

  createWallet(name, password) {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before creating wallet",
      );
    }
    if (password.length < this.config.minPasswordLength) {
      throw new Error(
        `Password must be at least ${this.config.minPasswordLength} characters long`,
      );
    }
    if (this.wallets.has(name)) {
      throw new Error(`Wallet with name ${name} already exists`);
    }

    const mnemonic = bip39.generateMnemonic();
    const wallet = new Wallet(name, mnemonic, password);

    // Generate initial addresses
    for (let i = 0; i < this.config.defaultAddressCount; i++) {
      wallet.generateAddress(i);
    }

    this.wallets.set(name, wallet);
    this.emit("walletCreated", { name });
    return wallet;
  }

  loadWallet(name, password) {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before loading wallet",
      );
    }
    if (!this.wallets.has(name)) {
      throw new Error(`Wallet ${name} does not exist`);
    }

    const wallet = this.wallets.get(name);
    if (wallet.password !== password) {
      throw new Error("Invalid password");
    }

    this.currentWallet = wallet;
    this.emit("walletLoaded", { name });
    return wallet;
  }

  unloadWallet() {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before unloading wallet",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }

    const name = this.currentWallet.name;
    this.currentWallet = null;
    this.emit("walletUnloaded", { name });
  }

  getCurrentWallet() {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before getting current wallet",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }
    return this.currentWallet;
  }

  generateAddress() {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before generating address",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }

    const nextIndex = this.currentWallet.addresses.size;
    const address = this.currentWallet.generateAddress(nextIndex);
    this.emit("addressGenerated", { address, index: nextIndex });
    return address;
  }

  getAddresses() {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before getting addresses",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }

    return this.currentWallet.getAddresses();
  }

  getBalance() {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before getting balance",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }

    return this.currentWallet.balance;
  }

  backupWallet(path) {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before backing up wallet",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }

    const backup = {
      name: this.currentWallet.name,
      mnemonic: this.currentWallet.mnemonic,
      addresses: this.currentWallet.getAddresses(),
      createdAt: this.currentWallet.createdAt,
      lastUsed: this.currentWallet.lastUsed,
    };

    // In a real implementation, this would write to a file
    this.emit("walletBackedUp", { path, name: this.currentWallet.name });
    return backup;
  }

  restoreWallet(backup, password) {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before restoring wallet",
      );
    }
    if (this.wallets.has(backup.name)) {
      throw new Error(`Wallet with name ${backup.name} already exists`);
    }

    const wallet = new Wallet(backup.name, backup.mnemonic, password);
    wallet.createdAt = backup.createdAt;
    wallet.lastUsed = backup.lastUsed;

    // Restore addresses
    backup.addresses.forEach((addr) => {
      wallet.addresses.set(addr.index, addr);
    });

    this.wallets.set(backup.name, wallet);
    this.emit("walletRestored", { name: backup.name });
    return wallet;
  }

  signTransaction(transaction) {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before signing transaction",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }

    // In a real implementation, this would sign the transaction
    // using the appropriate private key
    this.emit("transactionSigned", { transaction });
    return transaction;
  }

  verifyTransaction(transaction, signature) {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before verifying transaction",
      );
    }
    if (!this.currentWallet) {
      throw new Error("No wallet is currently loaded");
    }

    // In a real implementation, this would verify the transaction signature
    // using the appropriate public key
    this.emit("transactionVerified", { transaction, signature });
    return true;
  }

  listWallets() {
    if (!this.initialized) {
      throw new Error(
        "Wallet Manager must be initialized before listing wallets",
      );
    }

    return Array.from(this.wallets.values()).map((wallet) => ({
      name: wallet.name,
      balance: wallet.balance,
      addressCount: wallet.addresses.size,
      createdAt: wallet.createdAt,
      lastUsed: wallet.lastUsed,
    }));
  }
}

module.exports = WalletManager;
