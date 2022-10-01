local client = {}

client.CONSTS = {
	headerEnd = "\r\n\r\n",
	requEnd = "\r\n"
}

client.buffer = ""

client.GETResponse = [[
HTTP/1.1 200
Content-Type: application/json ]] .. client.CONSTS.headerEnd ..
[[{
	"id": 10,
	"name": "shirt",
	"color": "red",
	"price": "$23"
}
]] .. client.CONSTS.requEnd

function client:handleClient(buf, fd)
	self.buffer = self.buffer .. buf
	if buf:sub(-4) == client.CONSTS.headerEnd then
		print("Server is responding to ", self.buffer)
		client:sendResponse(fd)
		self.buffer = ""
	end
end

function client:sendResponse(fd)
        SendToNetworkSSL(client.GETResponse, fd)
end

function ReceivedFromNetwork(buf, fd)
	print("LUA ReceivedFromNetwork on " .. fd .. ": " .. buf)
	client:handleClient(buf, fd)
end

print("LUA: Lua Entry Loaded.")
