/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package harmonizer2;

import java.io.File;
import java.io.IOException;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileNameExtensionFilter;

/**
 *
 * @author peterolson
 */
public class Harmonizer2UI extends javax.swing.JFrame {
    private static int ERROR_MESSAGE;

    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(Harmonizer2UI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(Harmonizer2UI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(Harmonizer2UI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(Harmonizer2UI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        System.out.println("Dang.");
        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            @Override
            public void run() {
                new Harmonizer2UI().setVisible(true);
            }
        });
    }
    private String file_name;
    
    /**
     * Creates new form Harmonizer2UI
     */
    public Harmonizer2UI() {
        initComponents();
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jFileChooser1 = new javax.swing.JFileChooser();
        upload = new javax.swing.JPanel();
        upload_button = new javax.swing.JButton();
        configure = new javax.swing.JPanel();
        time_signature = new javax.swing.JLabel();
        beats_per_minute = new javax.swing.JLabel();
        key = new javax.swing.JLabel();
        time_signature_box = new javax.swing.JComboBox();
        beats_per_minute_field = new javax.swing.JTextField();
        key_box = new javax.swing.JComboBox();
        output = new javax.swing.JPanel();
        harmonize = new javax.swing.JButton();
        number_of_files = new javax.swing.JLabel();
        number_of_parts = new javax.swing.JLabel();
        speed = new javax.swing.JLabel();
        number_of_files_box = new javax.swing.JComboBox();
        number_of_parts_box = new javax.swing.JComboBox();
        speed_box = new javax.swing.JComboBox();
        title = new javax.swing.JLabel();

        jFileChooser1.setFileFilter(null);

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        upload.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "UPLOAD", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Impact", 0, 16))); // NOI18N

        upload_button.setText("Select File");
        upload_button.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                upload_buttonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout uploadLayout = new org.jdesktop.layout.GroupLayout(upload);
        upload.setLayout(uploadLayout);
        uploadLayout.setHorizontalGroup(
            uploadLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(uploadLayout.createSequentialGroup()
                .addContainerGap()
                .add(upload_button)
                .addContainerGap(244, Short.MAX_VALUE))
        );
        uploadLayout.setVerticalGroup(
            uploadLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(uploadLayout.createSequentialGroup()
                .addContainerGap()
                .add(upload_button)
                .addContainerGap(372, Short.MAX_VALUE))
        );

        configure.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "CONFIGURE", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Impact", 0, 16))); // NOI18N

        time_signature.setFont(new java.awt.Font("Lucida Grande", 0, 16)); // NOI18N
        time_signature.setText("Time Signature:");

        beats_per_minute.setFont(new java.awt.Font("Lucida Grande", 0, 16)); // NOI18N
        beats_per_minute.setText("Beats Per Minute:");

        key.setFont(new java.awt.Font("Lucida Grande", 0, 16)); // NOI18N
        key.setText("Key:");

        time_signature_box.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "4/4", "3/4" }));

        beats_per_minute_field.setText("20 to 200");

        key_box.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "F", "C", "G", "D", "A", "E", "B" }));

        org.jdesktop.layout.GroupLayout configureLayout = new org.jdesktop.layout.GroupLayout(configure);
        configure.setLayout(configureLayout);
        configureLayout.setHorizontalGroup(
            configureLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(configureLayout.createSequentialGroup()
                .add(21, 21, 21)
                .add(configureLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(key)
                    .add(beats_per_minute)
                    .add(time_signature))
                .add(59, 59, 59)
                .add(configureLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(time_signature_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(beats_per_minute_field, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(key_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(58, Short.MAX_VALUE))
        );
        configureLayout.setVerticalGroup(
            configureLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(configureLayout.createSequentialGroup()
                .addContainerGap()
                .add(configureLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(time_signature)
                    .add(time_signature_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(106, 106, 106)
                .add(configureLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(beats_per_minute)
                    .add(beats_per_minute_field, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(71, 71, 71)
                .add(configureLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(key)
                    .add(key_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(142, Short.MAX_VALUE))
        );

        output.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "OUTPUT", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Impact", 0, 16))); // NOI18N

        harmonize.setFont(new java.awt.Font("Lucida Grande", 0, 18)); // NOI18N
        harmonize.setText("Harmonize");
        harmonize.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                harmonizeActionPerformed(evt);
            }
        });

        number_of_files.setFont(new java.awt.Font("Lucida Grande", 0, 16)); // NOI18N
        number_of_files.setText("Number of Files:");

        number_of_parts.setFont(new java.awt.Font("Lucida Grande", 0, 16)); // NOI18N
        number_of_parts.setText("Number of Parts:");

        speed.setFont(new java.awt.Font("Lucida Grande", 0, 16)); // NOI18N
        speed.setText("Speed:");

        number_of_files_box.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "1", "2", "3" }));

        number_of_parts_box.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "1", "2", "3", "4" }));

        speed_box.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Fast", "Medium", "Slow" }));

        org.jdesktop.layout.GroupLayout outputLayout = new org.jdesktop.layout.GroupLayout(output);
        output.setLayout(outputLayout);
        outputLayout.setHorizontalGroup(
            outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(outputLayout.createSequentialGroup()
                .addContainerGap()
                .add(outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(outputLayout.createSequentialGroup()
                        .add(outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(number_of_files)
                            .add(speed)
                            .add(number_of_parts))
                        .add(32, 32, 32)
                        .add(outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(number_of_parts_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(speed_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(number_of_files_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addContainerGap(80, Short.MAX_VALUE))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, outputLayout.createSequentialGroup()
                        .add(0, 0, Short.MAX_VALUE)
                        .add(harmonize, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 154, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(84, 84, 84))))
        );
        outputLayout.setVerticalGroup(
            outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(outputLayout.createSequentialGroup()
                .add(6, 6, 6)
                .add(outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(number_of_files)
                    .add(number_of_files_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 114, Short.MAX_VALUE)
                .add(outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(number_of_parts)
                    .add(number_of_parts_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(66, 66, 66)
                .add(outputLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(speed)
                    .add(speed_box, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(97, 97, 97)
                .add(harmonize, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 37, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );

        title.setFont(new java.awt.Font("Impact", 0, 60)); // NOI18N
        title.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        title.setText("HARMONIZER");

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(upload, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(configure, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(output, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
            .add(title, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );

        layout.linkSize(new java.awt.Component[] {output, upload}, org.jdesktop.layout.GroupLayout.HORIZONTAL);

        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(title, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 60, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(output, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(configure, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(upload, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        layout.linkSize(new java.awt.Component[] {configure, output, upload}, org.jdesktop.layout.GroupLayout.VERTICAL);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void harmonizeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_harmonizeActionPerformed
        
        // Thanks to http://www.homeandlearn.co.uk/java/java_combo_boxes.html
        // for help with combo boxes
        String str_time_sig = (String)time_signature_box.getSelectedItem();
        // Thanks to http://ubuntuforums.org/showthread.php?t=983405 for help with getText
        String str_beats_per_minute_field = (String)beats_per_minute_field.getText();
        String str_key = (String)key_box.getSelectedItem();
        String str_num_files;    
        str_num_files = (String)number_of_files_box.getSelectedItem();
        String str_num_parts = (String)number_of_parts_box.getSelectedItem();
        String str_speed = (String)speed_box.getSelectedItem();
        
        String output1;
        //output1 = (" "+str_time_sig + " " + str_beats_per_minute_field + " " + str_key + " " + str_num_files + " " + str_num_parts + " " + str_speed+" "+file_name);
        output1 = "./import somewhere.wav output.xml 0 0 120 4 0 4";
        System.out.println(output1);
        
        
        // Thanks to http://www.coderanch.com/t/419192/java/java/Runtime-getRuntime-exec-String-command for Runtime.getRuntime().exec code
        try {
            //Process p = Runtime.getRuntime().exec("/Users/peterolson/HelloWorld"+output1);
            Process p = Runtime.getRuntime().exec(output1);
        } catch (IOException e)
        {
        }
        
           
    }//GEN-LAST:event_harmonizeActionPerformed

    
    private void upload_buttonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_upload_buttonActionPerformed
        
        
        // Thanks to http://java.about.com/od/javax_swing/a/Jfilechooser-Overview.htm for the next line of code:
        jFileChooser1.setFileFilter(new FileNameExtensionFilter("WAV Files", "wav"));
        
        // Thanks to http://netbeans-org.1045718.n5.nabble.com/How-can-I-add-a-File-Chooser-to-a-button-in-Matisse-td2905800.html
        // for the showOpenDialog code
        if(jFileChooser1.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            
            // Thanks to http://java.about.com/od/javax_swing/a/Jfilechooser-Overview.htm for the next line of code:
            File file = jFileChooser1.getSelectedFile();
            
            // Thanks to https://forums.oracle.com/forums/thread.jspa?threadID=1255148&tstart=25920
            // for the following if statement and error message
            if (file != null && !file.getName().toLowerCase().endsWith(".wav"))
            {
                JOptionPane.showMessageDialog(this,"Invalid file selected!","Error",Harmonizer2UI.ERROR_MESSAGE);
            }
            else
            {
                file_name = file.getPath();
                System.out.println(file_name);
            }
             
        } 
    }//GEN-LAST:event_upload_buttonActionPerformed

    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel beats_per_minute;
    private javax.swing.JTextField beats_per_minute_field;
    private javax.swing.JPanel configure;
    private javax.swing.JButton harmonize;
    private javax.swing.JFileChooser jFileChooser1;
    private javax.swing.JLabel key;
    private javax.swing.JComboBox key_box;
    private javax.swing.JLabel number_of_files;
    private javax.swing.JComboBox number_of_files_box;
    private javax.swing.JLabel number_of_parts;
    private javax.swing.JComboBox number_of_parts_box;
    private javax.swing.JPanel output;
    private javax.swing.JLabel speed;
    private javax.swing.JComboBox speed_box;
    private javax.swing.JLabel time_signature;
    private javax.swing.JComboBox time_signature_box;
    private javax.swing.JLabel title;
    private javax.swing.JPanel upload;
    private javax.swing.JButton upload_button;
    // End of variables declaration//GEN-END:variables
}
