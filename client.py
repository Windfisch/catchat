from private import *
from telethon import TelegramClient, events
import asyncio
import sys
from concurrent.futures import ThreadPoolExecutor

from dct import *


async def ainput(prompt: str = ""):
    with ThreadPoolExecutor(1, "AsyncInput", lambda x: print(x, end="", flush=True), (prompt,)) as executor:
        return (await asyncio.get_event_loop().run_in_executor(
            executor, sys.stdin.readline
        )).rstrip()


tg = TelegramClient('anon', api_id, api_hash)
tg.start()

def encrypt(key, x):
	return x # TODO

def decrypt(key, x):
	return x # TODO

async def main(tg, key):
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
				print("got image!")
				media = await event.message.download_media(file=bytes)
				print("image has length %i" % len(media))
				open("/tmp/recv.jpeg", 'wb').write(media)
				print("wrote image")
				print(decode_jpeg(media))
				print("---")
				print(decode_str(decrypt(key, decode_jpeg(media))))
				print("decrypted image")
			else:
				print("<<< message not understood.")


	while True:
		message = await ainput("Your message: ")
		bs = open("/home/flo/cat.jpeg", mode='rb').read()
		bs = encode_jpeg(bs, encrypt(key, encode_str(message)))
		await tg.send_file(dialog.entity, bs)

key = input("Enter your key (press enter to generate one): ")
if key == '':
	# TODO generate a key and print it
	pass
else:
	# TODO base64-decode the key
	pass

tg.loop.run_until_complete(main(tg, key))


