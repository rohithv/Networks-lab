// Java implementation of Server side 
// It contains two classes : Server and ClientHandler 
// Save file as Server.java 

import java.io.*; 
import java.text.*; 
import java.util.*; 
import java.net.*; 

// Server class 
public class Server 
{ 
	static ArrayList<Socket> clients;
	public static void main(String[] args) throws IOException 
	{ 
		// server is listening on port 5056 
		ServerSocket ss = new ServerSocket(5056); 
		clients = new ArrayList<Socket>();
		// running infinite loop for getting 
		// client request 
		while (true) 
		{ 
			Socket s = null; 
			
			try
			{ 
				// socket object to receive incoming client requests 
				s = ss.accept(); 
				
				System.out.println("A new client is connected : " + s); 
				clients.add(s);
				// obtaining input and out streams 
				DataInputStream dis = new DataInputStream(s.getInputStream()); 
				//DataOutputStream dos = new DataOutputStream(s.getOutputStream()); 
				
				System.out.println("Assigning new thread for this client"); 

				// create a new thread object 
				Thread t = new ClientHandler(s, dis); 

				// Invoking the start() method 
				t.start(); 
				
			} 
			catch (Exception e){ 
				s.close(); 
				e.printStackTrace(); 
			} 
		} 
	} 

	public static void sendText(String text, Socket soc){
		for(Socket i : clients){
			try{
				if(i!=soc){
					DataOutputStream dos = new DataOutputStream(i.getOutputStream());
					Thread t = new TextSender(i, dos, text);
					t.start();
				}
				
			}catch(Exception e){
				e.printStackTrace();
			}
		}
	}
	public static void rmsock(Socket s){
		int n = clients.indexOf(s);
		clients.remove(n);
	}
} 

// ClientHandler class 
class ClientHandler extends Thread 
{ 
	//DateFormat fordate = new SimpleDateFormat("yyyy/MM/dd"); 
	//DateFormat fortime = new SimpleDateFormat("hh:mm:ss"); 
	final DataInputStream dis; 
	//final DataOutputStream dos; 
	final Socket s; 
	

	// Constructor 
	public ClientHandler(Socket s, DataInputStream dis) 
	{ 
		this.s = s; 
		this.dis = dis; 
		//this.dos = dos; 
	} 

	@Override
	public void run() 
	{ 
		String received; 
		
		while (true) 
		{ 
			try { 

				// Ask user what he wants 
				/*dos.writeUTF("What do you want?[Date | Time]..\n"+ 
							"Type Exit to terminate connection."); */
				
				// receive the answer from client 
				received = dis.readUTF(); 
				
				if(received.equals("Exit")) 
				{ 
					System.out.println("Client " + this.s + " sends exit..."); 
					System.out.println("Closing this connection."); 
					Server.rmsock(s);
					this.s.close(); 
					System.out.println("Connection closed"); 
					break; 
				} 
				
				// creating Date object 
				//Date date = new Date(); 
				
				// write on output stream based on the 
				// answer from the client 
				//else --- BROADCAST
				Server.sendText(received.substring(9), s);
			} catch (IOException e) { 
				e.printStackTrace(); 
				break;
			} 
		} 
		
		try
		{ 
			// closing resources 
			this.dis.close(); 
			//this.dos.close(); 
			
		}catch(IOException e){ 
			e.printStackTrace(); 
		} 
	} 
} 

class TextSender extends Thread{
	final DataOutputStream dos; 
	final Socket s;
	final String toreturn; 

	public TextSender(Socket s, DataOutputStream dos, String toreturn){
		this.s = s;
		this.dos = dos;
		this.toreturn = toreturn;
	}

	@Override
	public void run(){
		try{
			dos.writeUTF(toreturn);
			//dos.close();
		}catch(Exception e){
			e.printStackTrace();
		}
	}
}