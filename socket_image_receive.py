
#!/usr/bin/env python3
import io
import socket

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432		# Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.bind((HOST, PORT))
	s.listen()
	conn, addr = s.accept()
	with conn:
		print('Connected by', addr)
		recv_size = 0
		filename = ''
		
		fdata = bytes()
		while True:
				data = conn.recv(1024)
				recv_size += len(data)
				if not data:
					break
				fdata += data;
#!第1字节固定为65,表示开头,第2,3,4,5为文件的长度, 6-45为文件名称, .... 图像数据.... 最后一个byte为checksum
#! checksum为单字节校验, 计算方法为 所有字节相加和, 取最低位的补码
		if recv_size > 45:
			pic_len = 0
			if fdata[0] == 65:
				pic_len = fdata[4] 
				pic_len <<= 8;
				pic_len |= fdata[3]
				pic_len <<= 8;
				pic_len |= fdata[2]
				pic_len <<= 8;
				pic_len |= fdata[1]
				
#!				提取文件名称
				for i in range(40): 
					if fdata[5+i] == 0:
						break
					filename += fdata[5+i:6+i].decode()

#!				计算checksum, 检查数据是否有错误
				checksum = 0
				for i in range(recv_size - 1): 
					checksum += fdata[i]
				checksum = checksum % 256;
				checksum = 256 - checksum;
				
#!				checksum计算结果与传送的checksum相同(最后一个byte),表示数据正确, 保存图片
				if checksum == fdata[recv_size-1]:
					print('checksum ok')
					f = open(filename, 'wb')
					f.write(fdata[45:recv_size-1])
					f.close()
				else:
					print('checksum error')
					print(checksum)
					print(fdata[recv_size-1])
					
			else:
				pic_len = recv_size
				filename = 'file.jpg'
				f = open(filename, 'wb')
				f.write(fdata)
				f.close()

		print('tcp receive:')
		print(recv_size)
		print('image size:')
		print(pic_len)

		conn.sendall('AB1234'.encode())

