#![feature(custom_derive, custom_attribute, plugin)]
#![plugin(diesel_codegen, dotenv_macros)]

extern crate serde_json;
extern crate websocket;
extern crate common;

use std::thread;
use std::str::from_utf8;
use websocket::{Server, Message, Sender, Receiver};
use websocket::result::WebSocketError;
use websocket::message::Type;
use websocket::header::WebSocketProtocol;

fn main() {
    let server = Server::bind("127.0.0.1:2794").unwrap();

	for connection in server {
		// Spawn a new thread for each connection.
		thread::spawn(move || {
			let request = connection.unwrap().read_request().unwrap(); // Get the request

			request.validate().unwrap(); // Validate the request

			let response = request.accept(); // Form a response

			let mut client = response.send().unwrap(); // Send the response

			let ip = client.get_mut_sender()
				.get_mut()
				.peer_addr()
				.unwrap();

			println!("Connection from {}", ip);

			let message: Message = Message::text("Hello".to_string());
			client.send_message(&message).unwrap();

			let (mut sender, mut receiver) = client.split();

			for message in receiver.incoming_messages() {
				let message: Message = match message {
                    Ok(val) => val,
                    Err(err) => {
                        match err {
                            WebSocketError::NoDataAvailable => {},
                            _ => println!("error: {:?}", err)
                        }
                        continue;
                    }
                };

				match message.opcode {
                    Type::Text => {
                        let payload = from_utf8(&*message.payload).unwrap();
                        println!("received: {:?}", payload);
                        let message: common::Message = match common::deserialize_message::<common::Message>(&payload) {
                            Ok(val) => val,
                            Err(err) => {
                                println!("error: {:?}", err);
                                continue;
                            }
                        };

                        match message.msg_type {
                            common::Type::Login => {
                                let login: common::LoginV1 = match common::deserialize_message(&message.content) {
                                    Ok(val) => val,
                                    Err(err) => {
                                        println!("error: {:?}", err);
                                        continue;
                                    }
                                };
                                println!("login received: {} - {}", login.username, login.password);
                            },
                            common::Type::Register => {
                                let register: common::RegisterV1 = match common::deserialize_message(&message.content) {
                                    Ok(val) => val,
                                    Err(err) => {
                                        println!("error: {:?}", err);
                                        continue;
                                    }
                                };
                                println!("register received: {} - {}", register.username, register.password);
                            }
                        }
                    },
					Type::Close => {
						let message = Message::close();
						sender.send_message(&message).unwrap();
						println!("Client {} disconnected", ip);
						return;
					},
					Type::Ping => {
						let message = Message::pong(message.payload);
						sender.send_message(&message).unwrap();
					}
					_ => sender.send_message(&message).unwrap(),
				}
			}
		});
}
}
