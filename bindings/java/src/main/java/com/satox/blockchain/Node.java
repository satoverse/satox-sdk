package com.satox.blockchain;

public class Node {
    private String id;
    private String host;
    private String port;

    public Node(String host, String port) {
        this.host = host;
        this.port = port;
        this.id = host + ":" + port;
    }

    public String getId() {
        return id;
    }

    public String getHost() {
        return host;
    }

    public String getPort() {
        return port;
    }
}