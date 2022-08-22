function ReceivedFromNetwork(buf, fd)
        print("From Lua: ", buf, fd)

        local str = [[{"data" : [ 0: Object { "name": "Zasya"},
                1: { "name": "Syake Sushi Hurghada"},
                "2": { "name": "Sofra", latitude: "26.988743", … }
                "3": { "name": "Cratus Premium Restaurant"}]} ]]

        print("Sending " .. str)
        SendToNetwork(str, fd)
end
