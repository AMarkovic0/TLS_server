function ReceivedFromNetwork(buf, fd)
        print("From Lua: ", buf, fd)
        SendToNetwork(buf, fd)
end
