from private import *
from telethon import TelegramClient, events
import asyncio
import sys
from concurrent.futures import ThreadPoolExecutor

from dct import *

import nacl.secret
import nacl.utils
import base64

def decrypt(secretbox, ciphertext):
	print (len(ciphertext))
	for i in range(len(ciphertext)-1, -1, -1):
		print(i)
		try:
			return secretbox.decrypt(ciphertext[0:i])
		except Exception as e:
			if i==49:
				print("wat")
				print(e)
			pass
	raise RuntimeError("failed to decrypt :(")

async def ainput(prompt: str = ""):
    with ThreadPoolExecutor(1, "AsyncInput", lambda x: print(x, end="", flush=True), (prompt,)) as executor:
        return (await asyncio.get_event_loop().run_in_executor(
            executor, sys.stdin.readline
        )).rstrip()


tg = TelegramClient('anon', api_id, api_hash)
tg.start()

async def main(tg, secretbox):
	dialogs = await tg.get_dialogs(limit=10)
	for (i,d) in enumerate(dialogs):
		print("%2i) %s" % (i, d.title))
	
	num = await ainput("Enter a dialog number: ")
	num = int(num)
	
	dialog = dialogs[num]
	print("selected %s" % num)

	did = dialog.id
	@tg.on(events.NewMessage)
	async def message_handler(event):
		print(event.chat_id, did)
		if (event.chat_id == did):
			if event.message.photo:
				try:
					print("got image!")
					media = await event.message.download_media(file=bytes)
					print("image has length %i" % len(media))
					open("/tmp/recv.jpeg", 'wb').write(media)
					print("wrote image")
					print(decode_jpeg(media))
					print("---")
					print(decode_str(decrypt(secretbox, decode_jpeg(media))))
					print("decrypted image")
				except Exception as e:
					print(e)
			else:
				print("<<< message not understood.")


	while True:
		message = await ainput("Your message: ")
		bs = open("/home/flo/meow2.jpg", mode='rb').read()
		secrettext = secretbox.encrypt(encode_str(message))
		print(len(secrettext))
		bs = encode_jpeg(bs, secrettext)
		await tg.send_file(dialog.entity, bs)

key = input("Enter your key (press enter to generate one): ")
if key == '':
	key = nacl.utils.random(nacl.secret.SecretBox.KEY_SIZE)
	print("Generated a key for you. It is: %s" % base64.b64encode(key).decode('ascii'))
else:
	key = base64.b64decode(key)

tg.loop.run_until_complete(main(tg, nacl.secret.SecretBox(key)))


