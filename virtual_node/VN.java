import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;


public class VN{
    public static void main(String[] args){
        if (args.length != 3) {
            System.err.println("Usage: java VN <oracle-ip> <vn-ip> <udp-port>");
            System.exit(1); //stop program
        }
        String oracleIp = args[0];
        String vnIp = args[1];
        int udpPort;
        try{
            udpPort=Integer.parseInt(args[2]);
            if (udpPort < 1 || udpPort > 65535) {
                throw new IllegalArgumentException(
                    "UDP port must be between 1 and 65535"
                );
            }
            //but like if its ensured that only the things are fine then its okay

            try(
                Socket socket=new Socket(oracleIp,5000)
                DataOutputStream out =new DataOutputStream(socket.getOutputStream());
                DataInputStream in =new DataInputStream(socket.getInputStream())
            ){
                byte[] ipBytes=InetAddress.getByName(vnIp).getAddress();
                out.write(ipBytes);
                out.writeShort(udpPort);
                out.flush();
                System.out.println("Sent CONNECT to Oracle Node");
                int tupleCount = in.readUnsignedShort();
                System.out.println("Received LINK-STATE with "+tupleCount+" tuples");
                StringBuilder neighbourhood = new StringBuilder();
                for (int i = 0; i < tupleCount; i++) {
                    char nodeId = (char) in.readUnsignedByte();
                    byte[] addressBytes = new byte[4];
                    in.readFully(addressBytes);
                    String ip = InetAddress.getByAddress(addressBytes)
                                           .getHostAddress();
                    int port = in.readUnsignedShort();
                    long cost = Integer.toUnsignedLong(in.readInt());
                    if (i > 0) {
                        neighbourhood.append(", ");
                    }
                    neighbourhood.append(nodeId).append("=").append(cost);
                    System.out.println("  Node " + nodeId+ ": IP=" + ip+ ", UDP port=" + port + ", cost=" + cost);
                }
                System.out.println("Neighbourhood: " + neighbourhood);
            }catch (Exception e) {
                System.err.println("VN error: " + e.getMessage());
                System.exit(1);
            }

        }
    }
}