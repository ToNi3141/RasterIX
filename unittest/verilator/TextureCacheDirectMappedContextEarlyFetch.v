`include "TextureCacheDirectMappedContext.v"

module TextureCacheDirectMappedContextEarlyFetch
(
    input wire                             aclk,
    input wire                             resetn,

    input wire                             s_tc_valid,
    output wire                            s_tc_ready,
    input wire                             s_tc_cmd,
    input wire [17 : 0]                    s_tc_addr,

    output wire [15 : 0]                   m_tc_texel,
    output wire                            m_tc_valid,
    input wire                             m_tc_ready,

    input wire [3 : 0]                     m_axi_rid,
    input wire [31 : 0]                    m_axi_rdata,
    input wire [1 : 0]                     m_axi_rresp,
    input wire                             m_axi_rlast,
    input wire                             m_axi_rvalid,
    output wire                            m_axi_rready
);
    TextureCacheDirectMappedContext #(
        .ENABLE_EARLY_FETCH(1)
    ) cache_context (
        .aclk(aclk),
        .resetn(resetn),
        .s_tc_valid(s_tc_valid),
        .s_tc_ready(s_tc_ready),
        .s_tc_cmd(s_tc_cmd),
        .s_tc_addr(s_tc_addr),
        .m_tc_texel(m_tc_texel),
        .m_tc_valid(m_tc_valid),
        .m_tc_ready(m_tc_ready),
        .m_axi_rid(m_axi_rid),
        .m_axi_rdata(m_axi_rdata),
        .m_axi_rresp(m_axi_rresp),
        .m_axi_rlast(m_axi_rlast),
        .m_axi_rvalid(m_axi_rvalid),
        .m_axi_rready(m_axi_rready)
    );
endmodule