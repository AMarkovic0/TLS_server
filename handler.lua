function ReceivedFromNetwork(buf, socket)
        print("FROM LUA: ", buf, socket)
        SendToNetwork(buf, socket)
end
