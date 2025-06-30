using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Satox.Bindings.IPFS;

namespace Satox.IPFS
{
    public class IPFSManager : IIPFSManager
    {
        private readonly IIPFSManager _manager;

        public IPFSManager(string apiUrl = "http://localhost:5001/api/v0", string gatewayUrl = "https://ipfs.io/ipfs/")
        {
            _manager = new Satox.Bindings.IPFS.IPFSManager(apiUrl, gatewayUrl);
        }

        public Task<string> StoreAsync(byte[] data) => _manager.StoreAsync(data);
        public Task<byte[]> RetrieveAsync(string cid) => _manager.RetrieveAsync(cid);
        public Task PinAsync(string cid) => _manager.PinAsync(cid);
        public Task UnpinAsync(string cid) => _manager.UnpinAsync(cid);
        public Task<List<string>> ListPinsAsync() => _manager.ListPinsAsync();
        public string GetGatewayUrl(string cid) => _manager.GetGatewayUrl(cid);
    }
} 