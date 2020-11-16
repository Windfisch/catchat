from private import *
from telethon import TelegramClient, events
import asyncio
import sys
from concurrent.futures import ThreadPoolExecutor


async def ainput(prompt: str = ""):
    with ThreadPoolExecutor(1, "AsyncInput", lambda x: print(x, end="", flush=True), (prompt,)) as executor:
        return (await asyncio.get_event_loop().run_in_executor(
            executor, sys.stdin.readline
        )).rstrip()


tg = TelegramClient('anon', api_id, api_hash)
tg.start()

async def main(tg):
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
				media = await event.message.download_media(file=bytes)
				message
				print(media)
			else:
				print("<<< message not understood: %s" % event.raw_text)


	while True:
		message = await ainput("Your message: ")
		await dialog.send_message(message)
		bs = open("/home/flo/cat.jpeg", mode='rb').read()
		print(bs)
		await tg.send_file(dialog.entity, bs)

tg.loop.run_until_complete(main(tg))


