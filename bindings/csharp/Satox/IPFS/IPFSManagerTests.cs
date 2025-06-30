using System;
using System.Net;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using Moq;
using Moq.Protected;
using Newtonsoft.Json;
using Xunit;

namespace Satox.IPFS.Tests
{
    public class IPFSManagerTests
    {
        private IPFSManager CreateManagerWithMock(HttpMessageHandler handler)
        {
            var httpClient = new HttpClient(handler);
            return new IPFSManager("http://localhost:5001/api/v0", "https://ipfs.io/ipfs/")
            {
                // For test injection, replace the internal HttpClient if needed
            };
        }

        [Fact]
        public async Task StoreAsync_ReturnsCid()
        {
            var handler = new Mock<HttpMessageHandler>();
            handler.Protected()
                .Setup<Task<HttpResponseMessage>>(
                    "SendAsync",
                    ItExpr.IsAny<HttpRequestMessage>(),
                    ItExpr.IsAny<CancellationToken>()
                )
                .ReturnsAsync(new HttpResponseMessage
                {
                    StatusCode = HttpStatusCode.OK,
                    Content = new StringContent("{\"Hash\":\"QmTestHash\"}")
                });

            var manager = CreateManagerWithMock(handler.Object);
            var cid = await manager.StoreAsync(new byte[] { 1, 2, 3 });
            Assert.Equal("QmTestHash", cid);
        }

        [Fact]
        public async Task RetrieveAsync_ReturnsData()
        {
            var handler = new Mock<HttpMessageHandler>();
            handler.Protected()
                .Setup<Task<HttpResponseMessage>>(
                    "SendAsync",
                    ItExpr.IsAny<HttpRequestMessage>(),
                    ItExpr.IsAny<CancellationToken>()
                )
                .ReturnsAsync(new HttpResponseMessage
                {
                    StatusCode = HttpStatusCode.OK,
                    Content = new ByteArrayContent(new byte[] { 4, 5, 6 })
                });

            var manager = CreateManagerWithMock(handler.Object);
            var data = await manager.RetrieveAsync("QmTestHash");
            Assert.Equal(new byte[] { 4, 5, 6 }, data);
        }

        [Fact]
        public async Task PinAsync_Succeeds()
        {
            var handler = new Mock<HttpMessageHandler>();
            handler.Protected()
                .Setup<Task<HttpResponseMessage>>(
                    "SendAsync",
                    ItExpr.IsAny<HttpRequestMessage>(),
                    ItExpr.IsAny<CancellationToken>()
                )
                .ReturnsAsync(new HttpResponseMessage { StatusCode = HttpStatusCode.OK });

            var manager = CreateManagerWithMock(handler.Object);
            await manager.PinAsync("QmTestHash");
        }

        [Fact]
        public async Task UnpinAsync_Succeeds()
        {
            var handler = new Mock<HttpMessageHandler>();
            handler.Protected()
                .Setup<Task<HttpResponseMessage>>(
                    "SendAsync",
                    ItExpr.IsAny<HttpRequestMessage>(),
                    ItExpr.IsAny<CancellationToken>()
                )
                .ReturnsAsync(new HttpResponseMessage { StatusCode = HttpStatusCode.OK });

            var manager = CreateManagerWithMock(handler.Object);
            await manager.UnpinAsync("QmTestHash");
        }

        [Fact]
        public async Task ListPinsAsync_ReturnsPins()
        {
            var pinsJson = JsonConvert.SerializeObject(new { Keys = new Dictionary<string, object> { { "QmTestHash", new { } } } });
            var handler = new Mock<HttpMessageHandler>();
            handler.Protected()
                .Setup<Task<HttpResponseMessage>>(
                    "SendAsync",
                    ItExpr.IsAny<HttpRequestMessage>(),
                    ItExpr.IsAny<CancellationToken>()
                )
                .ReturnsAsync(new HttpResponseMessage
                {
                    StatusCode = HttpStatusCode.OK,
                    Content = new StringContent(pinsJson)
                });

            var manager = CreateManagerWithMock(handler.Object);
            var pins = await manager.ListPinsAsync();
            Assert.Contains("QmTestHash", pins);
        }

        [Fact]
        public void GetGatewayUrl_ReturnsCorrectUrl()
        {
            var manager = new IPFSManager("http://localhost:5001/api/v0", "https://ipfs.io/ipfs/");
            var url = manager.GetGatewayUrl("QmTestHash");
            Assert.Equal("https://ipfs.io/ipfs/QmTestHash", url);
        }
    }
} 