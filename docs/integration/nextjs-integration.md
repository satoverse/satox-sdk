# Next.js Integration

## Overview

The Satox SDK provides seamless integration with Next.js applications, enabling developers to build modern web applications with blockchain functionality, quantum security, and real-time data synchronization.

## Installation

### Prerequisites
- Node.js 18+ 
- Next.js 13+ (App Router recommended)
- TypeScript (recommended)

### Install Satox SDK
```bash
# Install the Satox SDK
npm install @satox/satox-sdk

# Install additional dependencies for Next.js
npm install @satox/satox-sdk-nextjs
```

### Environment Setup
Create a `.env.local` file in your Next.js project root:

```env
# Satox SDK Configuration
SATOX_SDK_URL=http://localhost:8080
SATOX_SDK_API_KEY=your-api-key

# Database Configuration
DATABASE_URL=postgresql://user:password@localhost:5432/satox_db

# Cloud Services (Optional)
SUPABASE_URL=https://your-project.supabase.co
SUPABASE_ANON_KEY=your-supabase-anon-key
FIREBASE_PROJECT_ID=your-firebase-project-id
FIREBASE_API_KEY=your-firebase-api-key

# Authentication
NEXTAUTH_SECRET=your-nextauth-secret
NEXTAUTH_URL=http://localhost:3000
```

## Basic Setup

### SDK Initialization
Create a SDK configuration file:

```typescript
// lib/satox-sdk.ts
import { SatoxSDK } from '@satox/satox-sdk';
import { NextJSSDKAdapter } from '@satox/satox-sdk-nextjs';

// SDK Configuration
const sdkConfig = {
  url: process.env.SATOX_SDK_URL || 'http://localhost:8080',
  apiKey: process.env.SATOX_SDK_API_KEY,
  database: {
    url: process.env.DATABASE_URL,
    type: 'postgresql'
  },
  cloud: {
    supabase: {
      url: process.env.SUPABASE_URL,
      anonKey: process.env.SUPABASE_ANON_KEY
    },
    firebase: {
      projectId: process.env.FIREBASE_PROJECT_ID,
      apiKey: process.env.FIREBASE_API_KEY
    }
  }
};

// Initialize SDK
export const satoxSDK = new SatoxSDK(sdkConfig);

// Next.js specific adapter
export const nextJSAdapter = new NextJSSDKAdapter(satoxSDK);

// Initialize on app startup
export async function initializeSDK() {
  try {
    await satoxSDK.initialize();
    console.log('Satox SDK initialized successfully');
  } catch (error) {
    console.error('Failed to initialize Satox SDK:', error);
  }
}
```

### App Initialization
Initialize the SDK in your Next.js app:

```typescript
// app/layout.tsx
import { initializeSDK } from '@/lib/satox-sdk';

export default async function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  // Initialize SDK on app startup
  await initializeSDK();

  return (
    <html lang="en">
      <body>
        {children}
      </body>
    </html>
  );
}
```

## API Routes

### REST API Routes
Create API routes for SDK operations:

```typescript
// app/api/blockchain/route.ts
import { NextRequest, NextResponse } from 'next/server';
import { satoxSDK } from '@/lib/satox-sdk';

export async function GET(request: NextRequest) {
  try {
    const blockchainInfo = await satoxSDK.blockchain.getInfo();
    return NextResponse.json(blockchainInfo);
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to get blockchain info' },
      { status: 500 }
    );
  }
}

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const transaction = await satoxSDK.blockchain.createTransaction(body);
    return NextResponse.json(transaction);
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to create transaction' },
      { status: 500 }
    );
  }
}
```

### Wallet Management API
```typescript
// app/api/wallet/route.ts
import { NextRequest, NextResponse } from 'next/server';
import { satoxSDK } from '@/lib/satox-sdk';

export async function GET(request: NextRequest) {
  try {
    const wallets = await satoxSDK.wallet.listWallets();
    return NextResponse.json(wallets);
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to get wallets' },
      { status: 500 }
    );
  }
}

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const wallet = await satoxSDK.wallet.createWallet(body);
    return NextResponse.json(wallet);
  } catch (error) {
    return NextResponse.json(
      { error: 'Failed to create wallet' },
      { status: 500 }
    );
  }
}
```

### Database Operations API
```typescript
// app/api/database/route.ts
import { NextRequest, NextResponse } from 'next/server';
import { satoxSDK } from '@/lib/satox-sdk';

export async function GET(request: NextRequest) {
  try {
    const { searchParams } = new URL(request.url);
    const table = searchParams.get('table');
    const query = searchParams.get('query');
    
    const result = await satoxSDK.database.query(table!, query!);
    return NextResponse.json(result);
  } catch (error) {
    return NextResponse.json(
      { error: 'Database query failed' },
      { status: 500 }
    );
  }
}

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const result = await satoxSDK.database.insert(body.table, body.data);
    return NextResponse.json(result);
  } catch (error) {
    return NextResponse.json(
      { error: 'Database insert failed' },
      { status: 500 }
    );
  }
}
```

## React Components

### SDK Provider
Create a context provider for SDK access:

```typescript
// components/SatoxProvider.tsx
'use client';

import { createContext, useContext, ReactNode } from 'react';
import { satoxSDK } from '@/lib/satox-sdk';

const SatoxContext = createContext(satoxSDK);

export function SatoxProvider({ children }: { children: ReactNode }) {
  return (
    <SatoxContext.Provider value={satoxSDK}>
      {children}
    </SatoxContext.Provider>
  );
}

export function useSatox() {
  const context = useContext(SatoxContext);
  if (!context) {
    throw new Error('useSatox must be used within a SatoxProvider');
  }
  return context;
}
```

### Blockchain Status Component
```typescript
// components/BlockchainStatus.tsx
'use client';

import { useState, useEffect } from 'react';
import { useSatox } from './SatoxProvider';

export function BlockchainStatus() {
  const [status, setStatus] = useState<any>(null);
  const [loading, setLoading] = useState(true);
  const satox = useSatox();

  useEffect(() => {
    async function fetchStatus() {
      try {
        const blockchainStatus = await satox.blockchain.getStatus();
        setStatus(blockchainStatus);
      } catch (error) {
        console.error('Failed to fetch blockchain status:', error);
      } finally {
        setLoading(false);
      }
    }

    fetchStatus();
  }, [satox]);

  if (loading) {
    return <div>Loading blockchain status...</div>;
  }

  return (
    <div className="blockchain-status">
      <h3>Blockchain Status</h3>
      <div>Network: {status?.network}</div>
      <div>Block Height: {status?.blockHeight}</div>
      <div>Connected Peers: {status?.connectedPeers}</div>
      <div>Sync Status: {status?.syncStatus}</div>
    </div>
  );
}
```

### Wallet Management Component
```typescript
// components/WalletManager.tsx
'use client';

import { useState, useEffect } from 'react';
import { useSatox } from './SatoxProvider';

export function WalletManager() {
  const [wallets, setWallets] = useState<any[]>([]);
  const [loading, setLoading] = useState(true);
  const satox = useSatox();

  useEffect(() => {
    loadWallets();
  }, []);

  async function loadWallets() {
    try {
      const walletList = await satox.wallet.listWallets();
      setWallets(walletList);
    } catch (error) {
      console.error('Failed to load wallets:', error);
    } finally {
      setLoading(false);
    }
  }

  async function createWallet() {
    try {
      const newWallet = await satox.wallet.createWallet({
        name: `Wallet ${wallets.length + 1}`,
        type: 'quantum-resistant'
      });
      setWallets([...wallets, newWallet]);
    } catch (error) {
      console.error('Failed to create wallet:', error);
    }
  }

  if (loading) {
    return <div>Loading wallets...</div>;
  }

  return (
    <div className="wallet-manager">
      <h3>Wallet Manager</h3>
      <button onClick={createWallet}>Create New Wallet</button>
      <div className="wallets">
        {wallets.map((wallet) => (
          <div key={wallet.id} className="wallet">
            <h4>{wallet.name}</h4>
            <p>Address: {wallet.address}</p>
            <p>Balance: {wallet.balance} SATOX</p>
          </div>
        ))}
      </div>
    </div>
  );
}
```

### Transaction Component
```typescript
// components/TransactionForm.tsx
'use client';

import { useState } from 'react';
import { useSatox } from './SatoxProvider';

export function TransactionForm() {
  const [recipient, setRecipient] = useState('');
  const [amount, setAmount] = useState('');
  const [loading, setLoading] = useState(false);
  const satox = useSatox();

  async function sendTransaction(e: React.FormEvent) {
    e.preventDefault();
    setLoading(true);

    try {
      const transaction = await satox.blockchain.sendTransaction({
        recipient,
        amount: parseFloat(amount),
        walletId: 'default' // You would get this from wallet selection
      });

      alert(`Transaction sent! TX ID: ${transaction.id}`);
      setRecipient('');
      setAmount('');
    } catch (error) {
      console.error('Transaction failed:', error);
      alert('Transaction failed. Please try again.');
    } finally {
      setLoading(false);
    }
  }

  return (
    <form onSubmit={sendTransaction} className="transaction-form">
      <h3>Send Transaction</h3>
      <div>
        <label>Recipient Address:</label>
        <input
          type="text"
          value={recipient}
          onChange={(e) => setRecipient(e.target.value)}
          required
        />
      </div>
      <div>
        <label>Amount (SATOX):</label>
        <input
          type="number"
          value={amount}
          onChange={(e) => setAmount(e.target.value)}
          step="0.000001"
          required
        />
      </div>
      <button type="submit" disabled={loading}>
        {loading ? 'Sending...' : 'Send Transaction'}
      </button>
    </form>
  );
}
```

## Real-time Features

### WebSocket Integration
```typescript
// lib/websocket.ts
import { satoxSDK } from './satox-sdk';

export class SatoxWebSocket {
  private ws: WebSocket | null = null;
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 5;

  constructor(private url: string) {}

  connect() {
    this.ws = new WebSocket(this.url);

    this.ws.onopen = () => {
      console.log('WebSocket connected');
      this.reconnectAttempts = 0;
    };

    this.ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      this.handleMessage(data);
    };

    this.ws.onclose = () => {
      console.log('WebSocket disconnected');
      this.reconnect();
    };

    this.ws.onerror = (error) => {
      console.error('WebSocket error:', error);
    };
  }

  private handleMessage(data: any) {
    switch (data.type) {
      case 'blockchain_update':
        // Handle blockchain updates
        break;
      case 'transaction_confirmed':
        // Handle transaction confirmations
        break;
      case 'wallet_update':
        // Handle wallet updates
        break;
    }
  }

  private reconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++;
      setTimeout(() => {
        this.connect();
      }, 1000 * this.reconnectAttempts);
    }
  }

  send(data: any) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(data));
    }
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
    }
  }
}
```

### Real-time Hook
```typescript
// hooks/useRealTime.ts
'use client';

import { useState, useEffect } from 'react';
import { SatoxWebSocket } from '@/lib/websocket';

export function useRealTime() {
  const [isConnected, setIsConnected] = useState(false);
  const [lastUpdate, setLastUpdate] = useState<Date | null>(null);

  useEffect(() => {
    const ws = new SatoxWebSocket(process.env.NEXT_PUBLIC_WS_URL!);
    
    ws.connect();
    setIsConnected(true);

    return () => {
      ws.disconnect();
      setIsConnected(false);
    };
  }, []);

  return { isConnected, lastUpdate };
}
```

## Authentication Integration

### NextAuth.js Integration
```typescript
// app/api/auth/[...nextauth]/route.ts
import NextAuth from 'next-auth';
import { satoxSDK } from '@/lib/satox-sdk';

const handler = NextAuth({
  providers: [
    // Configure your providers here
  ],
  callbacks: {
    async signIn({ user, account, profile }) {
      // Create or update user in Satox database
      try {
        await satoxSDK.database.insert('users', {
          id: user.id,
          email: user.email,
          name: user.name,
          provider: account?.provider
        });
        return true;
      } catch (error) {
        console.error('Failed to create user:', error);
        return false;
      }
    },
    async session({ session, token }) {
      // Add Satox user data to session
      if (token.sub) {
        const user = await satoxSDK.database.query(
          'users',
          `SELECT * FROM users WHERE id = '${token.sub}'`
        );
        session.satoxUser = user[0];
      }
      return session;
    }
  }
});

export { handler as GET, handler as POST };
```

## Database Integration

### Prisma Integration
```typescript
// lib/prisma.ts
import { PrismaClient } from '@prisma/client';

const globalForPrisma = globalThis as unknown as {
  prisma: PrismaClient | undefined;
};

export const prisma = globalForPrisma.prisma ?? new PrismaClient();

if (process.env.NODE_ENV !== 'production') globalForPrisma.prisma = prisma;
```

### Database Operations
```typescript
// lib/database.ts
import { satoxSDK } from './satox-sdk';
import { prisma } from './prisma';

export class DatabaseService {
  // Use Satox SDK for blockchain data
  async getBlockchainData() {
    return await satoxSDK.database.query('blockchain_data', 'SELECT * FROM blockchain_data');
  }

  // Use Prisma for application data
  async getUserData(userId: string) {
    return await prisma.user.findUnique({
      where: { id: userId },
      include: { wallets: true, transactions: true }
    });
  }

  // Hybrid approach
  async getTransactionHistory(userId: string) {
    // Get user from Prisma
    const user = await prisma.user.findUnique({
      where: { id: userId }
    });

    // Get transactions from Satox SDK
    const transactions = await satoxSDK.blockchain.getTransactionHistory(user?.walletAddress);

    return transactions;
  }
}
```

## Cloud Integration

### Supabase Integration
```typescript
// lib/supabase.ts
import { createClient } from '@supabase/supabase-js';
import { satoxSDK } from './satox-sdk';

const supabaseUrl = process.env.NEXT_PUBLIC_SUPABASE_URL!;
const supabaseAnonKey = process.env.NEXT_PUBLIC_SUPABASE_ANON_KEY!;

export const supabase = createClient(supabaseUrl, supabaseAnonKey);

// Real-time subscriptions
export function subscribeToBlockchainUpdates(callback: (data: any) => void) {
  return supabase
    .channel('blockchain_updates')
    .on('postgres_changes', { event: '*', schema: 'public', table: 'transactions' }, callback)
    .subscribe();
}
```

### Firebase Integration
```typescript
// lib/firebase.ts
import { initializeApp } from 'firebase/app';
import { getFirestore, connectFirestoreEmulator } from 'firebase/firestore';
import { satoxSDK } from './satox-sdk';

const firebaseConfig = {
  apiKey: process.env.NEXT_PUBLIC_FIREBASE_API_KEY,
  authDomain: process.env.NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN,
  projectId: process.env.NEXT_PUBLIC_FIREBASE_PROJECT_ID,
  storageBucket: process.env.NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET,
  messagingSenderId: process.env.NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID,
  appId: process.env.NEXT_PUBLIC_FIREBASE_APP_ID
};

const app = initializeApp(firebaseConfig);
export const db = getFirestore(app);

// Connect to emulator in development
if (process.env.NODE_ENV === 'development') {
  connectFirestoreEmulator(db, 'localhost', 8080);
}
```

## Performance Optimization

### Server-Side Rendering
```typescript
// app/page.tsx
import { satoxSDK } from '@/lib/satox-sdk';

export default async function HomePage() {
  // Fetch data on server side
  const blockchainInfo = await satoxSDK.blockchain.getInfo();
  const recentTransactions = await satoxSDK.blockchain.getRecentTransactions();

  return (
    <div>
      <h1>Satox Blockchain</h1>
      <div>Block Height: {blockchainInfo.blockHeight}</div>
      <div>Recent Transactions: {recentTransactions.length}</div>
    </div>
  );
}
```

### Static Generation
```typescript
// app/blockchain/[blockId]/page.tsx
import { satoxSDK } from '@/lib/satox-sdk';

export async function generateStaticParams() {
  const blocks = await satoxSDK.blockchain.getRecentBlocks();
  
  return blocks.map((block) => ({
    blockId: block.id,
  }));
}

export default async function BlockPage({ params }: { params: { blockId: string } }) {
  const block = await satoxSDK.blockchain.getBlock(params.blockId);

  return (
    <div>
      <h1>Block {block.id}</h1>
      <div>Hash: {block.hash}</div>
      <div>Transactions: {block.transactions.length}</div>
    </div>
  );
}
```

## Error Handling

### Error Boundary
```typescript
// components/ErrorBoundary.tsx
'use client';

import { Component, ReactNode } from 'react';

interface Props {
  children: ReactNode;
}

interface State {
  hasError: boolean;
  error?: Error;
}

export class ErrorBoundary extends Component<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = { hasError: false };
  }

  static getDerivedStateFromError(error: Error): State {
    return { hasError: true, error };
  }

  componentDidCatch(error: Error, errorInfo: any) {
    console.error('Error caught by boundary:', error, errorInfo);
  }

  render() {
    if (this.state.hasError) {
      return (
        <div className="error-boundary">
          <h2>Something went wrong</h2>
          <p>Please try refreshing the page</p>
          <button onClick={() => this.setState({ hasError: false })}>
            Try Again
          </button>
        </div>
      );
    }

    return this.props.children;
  }
}
```

### API Error Handling
```typescript
// lib/api-error.ts
export class APIError extends Error {
  constructor(
    message: string,
    public statusCode: number,
    public code?: string
  ) {
    super(message);
    this.name = 'APIError';
  }
}

export function handleAPIError(error: any): APIError {
  if (error instanceof APIError) {
    return error;
  }

  if (error.response) {
    return new APIError(
      error.response.data?.message || 'API request failed',
      error.response.status,
      error.response.data?.code
    );
  }

  return new APIError('Network error', 500);
}
```

## Testing

### Unit Tests
```typescript
// __tests__/satox-sdk.test.ts
import { render, screen } from '@testing-library/react';
import { SatoxProvider } from '@/components/SatoxProvider';
import { BlockchainStatus } from '@/components/BlockchainStatus';

// Mock the SDK
jest.mock('@/lib/satox-sdk', () => ({
  satoxSDK: {
    blockchain: {
      getStatus: jest.fn().mockResolvedValue({
        network: 'testnet',
        blockHeight: 12345,
        connectedPeers: 5,
        syncStatus: 'synced'
      })
    }
  }
}));

describe('BlockchainStatus', () => {
  it('displays blockchain status', async () => {
    render(
      <SatoxProvider>
        <BlockchainStatus />
      </SatoxProvider>
    );

    expect(await screen.findByText('Blockchain Status')).toBeInTheDocument();
    expect(await screen.findByText('Network: testnet')).toBeInTheDocument();
  });
});
```

### Integration Tests
```typescript
// __tests__/api/blockchain.test.ts
import { createMocks } from 'node-mocks-http';
import { GET, POST } from '@/app/api/blockchain/route';

describe('/api/blockchain', () => {
  it('GET returns blockchain info', async () => {
    const { req, res } = createMocks({
      method: 'GET',
    });

    await GET(req);

    expect(res._getStatusCode()).toBe(200);
    const data = JSON.parse(res._getData());
    expect(data).toHaveProperty('network');
  });

  it('POST creates transaction', async () => {
    const { req, res } = createMocks({
      method: 'POST',
      body: {
        recipient: 'test-address',
        amount: 100
      },
    });

    await POST(req);

    expect(res._getStatusCode()).toBe(200);
    const data = JSON.parse(res._getData());
    expect(data).toHaveProperty('id');
  });
});
```

## Deployment

### Vercel Deployment
```json
// vercel.json
{
  "functions": {
    "app/api/**/*.ts": {
      "maxDuration": 30
    }
  },
  "env": {
    "SATOX_SDK_URL": "@satox-sdk-url",
    "SATOX_SDK_API_KEY": "@satox-sdk-api-key",
    "DATABASE_URL": "@database-url"
  }
}
```

### Docker Deployment
```dockerfile
# Dockerfile
FROM node:18-alpine

WORKDIR /app

COPY package*.json ./
RUN npm ci --only=production

COPY . .

RUN npm run build

EXPOSE 3000

CMD ["npm", "start"]
```

---

**Last Updated**: $(date '+%Y-%m-%d %H:%M:%S')
**Next.js Integration Version**: 1.0.0 