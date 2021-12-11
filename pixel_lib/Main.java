import java.net.*;
import java.io.*;
import java.util.Random;

public class Main {

    private static Socket socket;
    private static String ip = "172.27.164.47";
    private static int port = 7890;
    private static PrintWriter out;
    private static BufferedReader in;
    private static OutputStream output;

    public static void main(String[] args) {

        try {
            socket = new Socket(ip, port);
            System.out.println(" connected to " + socket.toString());
            out = new PrintWriter(socket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            output = socket.getOutputStream();

            while (true) {
                sendMessage();
            }

        } catch (IOException e) {
            e.printStackTrace();
        }

    }


    public static void sendMessage() {

        Random random = new Random();
        Pixel[] pixels = new Pixel[5];
        for (int i = 0; i < pixels.length; i++) {
            int r = random.nextInt(255);
            int g = random.nextInt(255);
            int b = random.nextInt(255);
            pixels[i] = new Pixel(r,g,b);
        }
        byte[] data = PixelLib.getByteArrayChannel(0,pixels);

        try {
            System.out.println("----------------------");
            for (int i = 0; i < data.length; i++) {
                System.out.println(data[i]);
            }
            output.write(data);
        } catch (IOException e) {
            e.printStackTrace();
        }

    }


    private static void sendPixel(){
        Pixel p = new Pixel(0,0,255);
    }

}
