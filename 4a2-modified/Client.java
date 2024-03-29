// Java implementation for a client 
// Save file as Client.java 

import java.io.*; 
import java.net.*; 
import java.text.*;
import java.util.*; 

// Client class 
public class Client 
{ 
	public static void main(String[] args) throws IOException 
	{ 
		try
		{ 
			Scanner scn = new Scanner(System.in); 
			
			// getting localhost ip 
			InetAddress ip = InetAddress.getByName("localhost"); 
	
			// establish the connection with server port 5056 
			Socket s = new Socket(ip, 5056); 
	
			// obtaining input and out streams 
			//DataInputStream dis = new DataInputStream(s.getInputStream()); 
			DataOutputStream dos = new DataOutputStream(s.getOutputStream()); 
			Thread t = new RecvThread(s);
			t.start();
			// the following loop performs the exchange of 
			// information between client and client handler 
			while (true) 
			{ 
				//System.out.println(dis.readUTF()); 
				String tosend = scn.nextLine(); 
				//dos.writeUTF(tosend); 
				
				// If client sends exit,close this connection 
				// and then break from the while loop 
				if(tosend.equals("Exit")) 
				{ 
					dos.writeUTF(tosend);
					System.out.println("Closing this connection : " + s); 
					s.close(); 
					System.out.println("Connection closed"); 
					break; 
				} 
				if(!tosend.startsWith("BROADCAST")){
					System.out.println("Unknown Command received");
					continue;
				}
				dos.writeUTF(tosend);
				// printing date or time as requested by client 
				//String received = dis.readUTF(); 
				//System.out.println(received); 
			} 
			
			// closing resources 
			scn.close(); 
			//dis.close(); 
			dos.close(); 
		}catch(Exception e){ 
			e.printStackTrace(); 
		} 
	} 
} 

class RecvThread extends Thread{
	final Socket s;
	DataInputStream dis; 
	public RecvThread(Socket sock){
		this.s = sock;
		try{
			this.dis = new DataInputStream(s.getInputStream());
		}catch(Exception e){
			e.printStackTrace();
		}
		
	}

	@Override
	public void run(){
		try{
			while(true){
				String received = dis.readUTF();
				System.out.println(received);
			}
		}catch(Exception e){
			//e.printStackTrace();
		}
		try{
			s.close();
			dis.close();
		}
		catch(Exception e){
			//e.printStackTrace();
		}
		
	}
}