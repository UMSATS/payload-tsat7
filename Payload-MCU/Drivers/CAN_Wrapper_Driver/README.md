# CAN Wrapper

This module wraps a simpler interface around HAL's CAN interface for sending & receiving messages onboard the TSAT satellite.

## Importing

You will need CAN set up in order to use this module.

To set up CAN, you will need to:

1. Enable a CAN peripheral in your `.ioc` file under `Pinout & Configuration > Connectivity`.
2. Enable `RX0 interrupt` in `CAN1 > NVIC Settings`.
3. Regenerate code.
4. Ensure initialization code has been generated in `main.c`

To import CAN Wrapper you will need to:

1. Copy this folder into your HAL project under `Drivers/`.
2. From STM32CubeIDE, right click on the `CAN_Wrapper_Driver/Inc` folder and click `Add/remove include path...`.
3. Leave all configurations selected and hit `OK`.
4. Call `CANWrapper_Init` somewhere after CAN is initialized (you will need to include `can_wrapper.h`).
5. Pass in a pointer to the CAN peripheral you are using (typically CAN1).

## Initialisation

This driver uses a flexible callback approach to handling incoming messages.

Pass in a reference to your message handling function when calling `CANWrapper_Init`:

```c
CANWrapper_Init(&hcan1, 0x2, &on_message_received);
```

Here's a template message handler to start:

```c
void on_message_received(CANMessage msg)
{
	CANMessageBody response_body = { .data = { msg.command_id } };

	// the reset command is a special case. (you must send a response FIRST)
	if (msg.command_id == CMD_RESET)
	{
		CANWrapper_Send_Response(true, response_body);
		Perform_Manual_Reset(); // implement this as per your subsystem.
	}

	HAL_StatusTypeDef status = HAL_OK; // use an appropriate enum in your case.
	uint8_t response_data_size = 0;

	switch (msg.command_id)
	{
		case CMD_LED_ON: // example command.
		{
			status = LEDs_Set_LED(msg.data[1], ON);
			response_body.data[1] = msg.data[1];
			response_data_size = 1;
			break;
		}
		// ...
		default:
		{
			status = HAL_ERROR;
			break;
		}
	}

	if (status != HAL_OK && response_data_size < CAN_MESSAGE_LENGTH-1)
	{
		response_body.data[response_data_size+1] = status; // append error code to NACK.
	}

	CANWrapper_Send_Response(status == HAL_OK, response_body); // return to sender.
}
```

## Additional Examples

### Create & Send a CAN Message

```c
#define CDH_ID               0x01
#define CMD_HELLO_MESSAGE    0xA3

void Say_Hello()
{
	CANMessage my_message = {
	    .recipient_id = CDH_ID,
	    .priority = 3,
	    .command_id = CMD_HELLO_MESSAGE,
	    .body = {
	        .data = {
	            'H', 'e', 'l', 'l', 'o', '\0'
	        }
	    }
	};
	
	CANWrapper_Send_Message(my_message);
}
```

### Accessing `CANMessage` Data Members

The `CANMessage` type is designed to be flexible in that you can use any of the fields:

 - `.command_id`
 - `.body`
 - `.data`

to access different parts of a CAN message.

```c
CANMessage msg;

msg.command_id = 0xB5;  // set command ID to 0xB5.
msg.data[0] = 0xB6;     // another way to set command ID (this time to 0xB6).

msg.body.data[0] = 'A'; // set first byte in message body. (ie. the byte after command ID)
msg.data[1] = 'A';      // equivalent.

CANMessageBody my_body = { .data = { 'H', 'i', '\0' } };
msg.body = my_body;     // replace the entire message body struct.
```

See `can_message.h` for the full structure definition.

## More Reading

The below link is old, so I don't recommend reading it, but I will leave it here in case you want to read the old documentation of this interface or you want to understand the high level concepts of how CAN works.

<https://drive.google.com/file/d/1HHNWpN6vo-JKY5VvzY14uecxMsGIISU7/view?usp=share_link>
