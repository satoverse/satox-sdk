using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Threading.Tasks;
using System.Text;
using Newtonsoft.Json;

namespace Satox.Bindings.IPFS
{
    public interface IIPFSManager : IDisposable
    {
        void Initialize();
        void Shutdown();
        Task<string> StoreAsync(byte[] data);
        Task<byte[]> RetrieveAsync(string cid);
        Task PinAsync(string cid);
        Task UnpinAsync(string cid);
        Task<List<string>> ListPinsAsync();
        string GetGatewayUrl(string cid);
    }

    public class IPFSManager : ThreadSafeManager, IIPFSManager
    {
        private readonly string _apiUrl;
        private readonly string _gatewayUrl;
        private readonly HttpClient _httpClient;

        public IPFSManager(string apiUrl = "http://localhost:5001/api/v0", string gatewayUrl = "https://ipfs.io/ipfs/")
            : base()
        {
            _apiUrl = apiUrl.TrimEnd('/');
            _gatewayUrl = gatewayUrl.TrimEnd('/') + "/";
            _httpClient = new HttpClient();
        }

        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                // Test connection to IPFS API
                try
                {
                    var testTask = _httpClient.GetAsync($"{_apiUrl}/version");
                    testTask.Wait(5000); // 5 second timeout
                    if (testTask.Result.IsSuccessStatusCode)
                    {
                        SetInitialized(true);
                    }
                    else
                    {
                        throw new SatoxError("Failed to connect to IPFS API");
                    }
                }
                catch (Exception ex)
                {
                    throw new SatoxError($"Failed to initialize IPFS manager: {ex.Message}");
                }
            }
        }

        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                _httpClient?.Dispose();
                SetInitialized(false);
            }
        }

        public async Task<string> StoreAsync(byte[] data)
        {
            EnsureInitialized();

            using (AcquireReadLock())
            {
                using (var content = new MultipartFormDataContent())
                {
                    content.Add(new ByteArrayContent(data), "file", "file.bin");
                    var response = await _httpClient.PostAsync($"{_apiUrl}/add", content);
                    response.EnsureSuccessStatusCode();
                    var json = await response.Content.ReadAsStringAsync();
                    dynamic result = JsonConvert.DeserializeObject(json);
                    return result.Hash;
                }
            }
        }

        public async Task<byte[]> RetrieveAsync(string cid)
        {
            EnsureInitialized();

            using (AcquireReadLock())
            {
                var response = await _httpClient.GetAsync($"{_gatewayUrl}{cid}");
                response.EnsureSuccessStatusCode();
                return await response.Content.ReadAsByteArrayAsync();
            }
        }

        public async Task PinAsync(string cid)
        {
            EnsureInitialized();

            using (AcquireReadLock())
            {
                var response = await _httpClient.PostAsync($"{_apiUrl}/pin/add?arg={cid}", null);
                response.EnsureSuccessStatusCode();
            }
        }

        public async Task UnpinAsync(string cid)
        {
            EnsureInitialized();

            using (AcquireReadLock())
            {
                var response = await _httpClient.PostAsync($"{_apiUrl}/pin/rm?arg={cid}", null);
                response.EnsureSuccessStatusCode();
            }
        }

        public async Task<List<string>> ListPinsAsync()
        {
            EnsureInitialized();

            using (AcquireReadLock())
            {
                var response = await _httpClient.PostAsync($"{_apiUrl}/pin/ls", null);
                response.EnsureSuccessStatusCode();
                var json = await response.Content.ReadAsStringAsync();
                dynamic result = JsonConvert.DeserializeObject(json);
                var pins = new List<string>();
                foreach (var pin in result.Keys)
                {
                    pins.Add(pin.ToString());
                }
                return pins;
            }
        }

        public string GetGatewayUrl(string cid)
        {
            EnsureInitialized();
            return $"{_gatewayUrl}{cid}";
        }

        protected override void DisposeCore()
        {
            if (IsInitialized)
                Shutdown();
        }
    }
}