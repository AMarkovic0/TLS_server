local client = {}

client.CONSTS = {
	requEnd = "\r\n\r\n",
}

client.buffer = ""

function client:readChunk(chunkStr)
	self.buffer = self.buffer .. chunkStr

	if chunkStr == self.CONSTS.requEnd then
		self:handleClientRequest()
	end
end

function client:handleClientRequest()
	print(self.buffer)
	self.buffer = ""
end

function ReceivedFromNetwork(buf, fd)
	client:readChunk(buf)
        --print("Sending " .. str)
        --SendToNetwork(str, fd)
end
